#include "app/DeviceWatcher.hpp"

#include <array>
#include <string>
#include <vector>

// ── Platform-specific headers ────────────────────────────────────────────────
#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
// winsock2 must be included before windows.h to avoid winsock1 symbol conflicts
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <windows.h>
#endif

namespace nanohawk::app {

// ── Shared protocol constants ─────────────────────────────────────────────────
//
// These must match firmware/companion_mcu/protocol.h.
// Only sent to devices NOT already identified by VID/PID.
//
//   Serial handshake : agent sends kSerialProbe (6 B), MCU replies kSerialAck (6 B)
//   WiFi discovery   : agent broadcasts kWifiProbe (14 B), MCU replies kWifiAck (10 B)
//
namespace {

constexpr std::array<char, 6> kSerialProbe = {'N','H','A','W','K','?'};
constexpr std::array<char, 6> kSerialAck   = {'N','H','A','W','K','!'};
constexpr char                kWifiProbe[] = "NHAWK_DISCOVER"; // 14 chars, no null
constexpr char                kWifiAck[]   = "NHAWK_FOUND";   // 10 chars, no null
constexpr int                 kWifiAckLen  = 10;

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
//  Windows implementation
// ─────────────────────────────────────────────────────────────────────────────
#if defined(_WIN32)

namespace {

// ── Known USB drone device table ─────────────────────────────────────────────
//
// Devices in this table are returned immediately when their VID/PID is found
// in the registry — no NHAWK? handshake required.  The baud field is the
// recommended opening baud for the associated serial protocol.
//
struct KnownDevice {
    const char* vid;         // e.g. "0483"
    const char* pid;         // e.g. "5740"
    const char* description; // human-readable label
    int         baud;        // recommended baud rate
};

constexpr std::array<KnownDevice, 6> kKnownDevices = {{
    // STM32 USB Virtual COM Port — Betaflight / Cleanflight flight controllers
    // (EMAX Nanohawk 1S stock FC, most other Betaflight boards)
    { "0483", "5740", "STM32 Betaflight FC (USB CDC)", 115200 },

    // Raspberry Pi RP2040 USB CDC — Nanohawk companion MCU firmware
    { "2E8A", "000A", "RP2040 Nanohawk companion MCU", 500000 },

    // Silicon Labs CP2102/CP2104 — common on many ESC/FC adapters
    { "10C4", "EA60", "CP210x USB-Serial", 115200 },

    // WCH CH340/CH341 — common cheap USB-serial chip
    { "1A86", "7523", "CH340 USB-Serial", 115200 },

    // FTDI FT232R — used on some older flight controller adapters
    { "0403", "6001", "FTDI FT232R USB-Serial", 115200 },

    // STM32 DFU bootloader — same VID, different PID; listed to avoid confusion
    { "0483", "DF11", "STM32 DFU Bootloader (not usable)", 0 },
}};

// ── Registry helpers ──────────────────────────────────────────────────────────

// Returns the COM port name (e.g. "COM3") for the first USB device matching
// vid:pid found under HKLM\SYSTEM\CurrentControlSet\Enum\USB\VID_xxxx&PID_xxxx.
// Returns "" when the device is not present.
std::string comPortForVidPid(const std::string& vid, const std::string& pid) {
    // Build the parent key path, e.g.  USB\VID_0483&PID_5740
    const std::string parentPath =
        "SYSTEM\\CurrentControlSet\\Enum\\USB\\VID_"
        + vid + "&PID_" + pid;

    HKEY hParent = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, parentPath.c_str(),
                      0, KEY_READ, &hParent) != ERROR_SUCCESS) {
        return {};
    }

    // Each subkey is an instance (serial number or index).
    char instanceName[256];
    DWORD instanceIdx = 0;
    std::string result;

    while (result.empty()) {
        DWORD nameSize = static_cast<DWORD>(sizeof(instanceName));
        const LONG rc  = RegEnumKeyExA(hParent, instanceIdx++,
                                       instanceName, &nameSize,
                                       nullptr, nullptr, nullptr, nullptr);
        if (rc == ERROR_NO_MORE_ITEMS) { break; }
        if (rc != ERROR_SUCCESS)       { continue; }

        // Open <instance>\Device Parameters and read PortName.
        const std::string paramPath =
            parentPath + "\\" + instanceName + "\\Device Parameters";

        HKEY hParam = nullptr;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, paramPath.c_str(),
                          0, KEY_READ, &hParam) != ERROR_SUCCESS) {
            continue;
        }

        char portName[64];
        DWORD portSize = static_cast<DWORD>(sizeof(portName));
        DWORD type     = 0;
        if (RegQueryValueExA(hParam, "PortName", nullptr, &type,
                             reinterpret_cast<LPBYTE>(portName),
                             &portSize) == ERROR_SUCCESS && type == REG_SZ) {
            result = portName; // e.g. "COM3"
        }
        RegCloseKey(hParam);
    }

    RegCloseKey(hParent);
    return result;
}

// Returns every COM port name registered in the Windows device map.
std::vector<std::string> enumerateComPorts() {
    std::vector<std::string> ports;
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DEVICEMAP\\SERIALCOMM",
                      0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return ports;
    }
    char  valueName[256];
    char  valueData[256];
    DWORD index = 0;
    while (true) {
        DWORD nameSize = static_cast<DWORD>(sizeof(valueName));
        DWORD dataSize = static_cast<DWORD>(sizeof(valueData));
        DWORD type     = 0;
        const LONG rc  = RegEnumValueA(hKey, index++,
                                       valueName, &nameSize,
                                       nullptr, &type,
                                       reinterpret_cast<LPBYTE>(valueData),
                                       &dataSize);
        if (rc == ERROR_NO_MORE_ITEMS) { break; }
        if (rc == ERROR_SUCCESS && type == REG_SZ) {
            ports.emplace_back(valueData);
        }
    }
    RegCloseKey(hKey);
    return ports;
}

// ── Serial handshake (companion MCU firmware only) ───────────────────────────

// Opens a COM port and exchanges the NHAWK? / NHAWK! handshake.
// Returns true only when the MCU responds with the expected bytes.
bool probeSerialHandshake(const std::string& portName, int baud) {
    const std::string path = "\\\\.\\" + portName;
    HANDLE hPort = CreateFileA(path.c_str(),
                               GENERIC_READ | GENERIC_WRITE,
                               0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPort == INVALID_HANDLE_VALUE) { return false; }

    DCB dcb{};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hPort, &dcb)) { CloseHandle(hPort); return false; }
    dcb.BaudRate = static_cast<DWORD>(baud);
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary  = TRUE;
    dcb.fParity  = FALSE;
    if (!SetCommState(hPort, &dcb)) { CloseHandle(hPort); return false; }

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout        = 50;
    to.ReadTotalTimeoutMultiplier = 2;
    to.ReadTotalTimeoutConstant   = 200;
    to.WriteTotalTimeoutConstant  = 100;
    SetCommTimeouts(hPort, &to);
    PurgeComm(hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);

    DWORD written = 0;
    if (!WriteFile(hPort, kSerialProbe.data(),
                   static_cast<DWORD>(kSerialProbe.size()),
                   &written, nullptr)) {
        CloseHandle(hPort); return false;
    }

    std::array<char, 6> response{};
    DWORD bytesRead = 0;
    ReadFile(hPort, response.data(),
             static_cast<DWORD>(response.size()), &bytesRead, nullptr);

    CloseHandle(hPort);
    return (bytesRead == kSerialAck.size()) && (response == kSerialAck);
}

// ── Main serial scan ──────────────────────────────────────────────────────────
//
// Detection priority:
//   1. Scan kKnownDevices by VID/PID (registry, no handshake needed).
//      Skip entries with baud==0 (bootloaders, not connectable).
//   2. For every remaining COM port not already matched, send NHAWK? handshake.
//
DetectedDevice scanSerial(int fallbackBaud) {
    // ── Stage 1: VID/PID priority scan ───────────────────────────────────────
    for (const auto& dev : kKnownDevices) {
        if (dev.baud == 0) { continue; } // bootloader, skip

        const std::string port = comPortForVidPid(dev.vid, dev.pid);
        if (port.empty()) { continue; }

        DetectedDevice d;
        d.transport   = DetectedDevice::Transport::Serial;
        d.portOrHost  = port;
        d.baudOrPort  = dev.baud;
        d.endpoint    = "serial://" + port + ":" + std::to_string(dev.baud);
        d.description = std::string(dev.description) + " on " + port;
        return d;
    }

    // ── Stage 2: NHAWK? handshake on all remaining ports ─────────────────────
    for (const auto& port : enumerateComPorts()) {
        if (probeSerialHandshake(port, fallbackBaud)) {
            DetectedDevice d;
            d.transport   = DetectedDevice::Transport::Serial;
            d.portOrHost  = port;
            d.baudOrPort  = fallbackBaud;
            d.endpoint    = "serial://" + port + ":" + std::to_string(fallbackBaud);
            d.description = "Nanohawk companion MCU on " + port
                          + " @ " + std::to_string(fallbackBaud) + " baud";
            return d;
        }
    }

    return {};
}

// ── WiFi UDP broadcast discovery ─────────────────────────────────────────────

DetectedDevice scanWifi(int discoveryPort, int timeoutMs) {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return {}; }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) { WSACleanup(); return {}; }

    constexpr int kOn = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
               reinterpret_cast<const char*>(&kOn), sizeof(kOn));

    const DWORD recvTimeout = static_cast<DWORD>(timeoutMs);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const char*>(&recvTimeout), sizeof(recvTimeout));

    sockaddr_in bindAddr{};
    bindAddr.sin_family      = AF_INET;
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_port        = 0;
    bind(sock, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr));

    sockaddr_in destAddr{};
    destAddr.sin_family      = AF_INET;
    destAddr.sin_addr.s_addr = INADDR_BROADCAST;
    destAddr.sin_port        = htons(static_cast<u_short>(discoveryPort));
    sendto(sock, kWifiProbe, static_cast<int>(sizeof(kWifiProbe) - 1), 0,
           reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));

    char        buf[64]{};
    sockaddr_in senderAddr{};
    int         senderLen = static_cast<int>(sizeof(senderAddr));
    const int   received  = recvfrom(sock, buf, static_cast<int>(sizeof(buf)) - 1,
                                     0,
                                     reinterpret_cast<sockaddr*>(&senderAddr),
                                     &senderLen);
    closesocket(sock);
    WSACleanup();

    if (received < kWifiAckLen) { return {}; }
    for (int i = 0; i < kWifiAckLen; ++i) {
        if (buf[i] != kWifiAck[i]) { return {}; }
    }

    char senderIp[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &senderAddr.sin_addr, senderIp, sizeof(senderIp));

    const std::string ip(senderIp);
    DetectedDevice d;
    d.transport   = DetectedDevice::Transport::WiFi;
    d.portOrHost  = ip;
    d.baudOrPort  = discoveryPort;
    d.endpoint    = "udp://" + ip + ":" + std::to_string(discoveryPort);
    d.description = "Nanohawk companion MCU via WiFi at "
                  + ip + ":" + std::to_string(discoveryPort);
    return d;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
//  Non-Windows stubs
// ─────────────────────────────────────────────────────────────────────────────
#else

namespace {
DetectedDevice scanSerial(int)           { return {}; }
DetectedDevice scanWifi(int, int)        { return {}; }
} // namespace

#endif // _WIN32

// ─────────────────────────────────────────────────────────────────────────────
//  Public DeviceWatcher methods
// ─────────────────────────────────────────────────────────────────────────────

DetectedDevice DeviceWatcher::detectSerial(int baud) const {
    return scanSerial(baud);
}

DetectedDevice DeviceWatcher::detectWifi(int discoveryPort, int timeoutMs) const {
    return scanWifi(discoveryPort, timeoutMs);
}

DetectedDevice DeviceWatcher::detectAny(int discoveryPort,
                                        int serialBaud,
                                        int timeoutMs) const {
    // WiFi: single UDP round-trip, try first.
    if (DetectedDevice d = scanWifi(discoveryPort, timeoutMs); d.found()) {
        return d;
    }
    // USB serial: VID/PID priority then NHAWK? handshake.
    return scanSerial(serialBaud);
}

bool DeviceWatcher::detectVehicleEndpoint() const {
    return detectAny().found();
}

bool DeviceWatcher::detectVideoEndpoint() const {
    return true; // handled by VideoSource
}

} // namespace nanohawk::app
