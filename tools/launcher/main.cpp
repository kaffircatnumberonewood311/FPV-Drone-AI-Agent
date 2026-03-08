#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
// winsock2 before windows.h to avoid winsock1 symbol conflicts
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// ── Protocol constants (must match DeviceWatcher.cpp and companion firmware) ──
namespace {

constexpr std::array<char, 6> kSerialProbe = {'N','H','A','W','K','?'};
constexpr std::array<char, 6> kSerialAck   = {'N','H','A','W','K','!'};
constexpr char                kWifiProbe[] = "NHAWK_DISCOVER"; // 14 chars, no null
constexpr char                kWifiAck[]   = "NHAWK_FOUND";   // 10 chars, no null
constexpr int                 kWifiAckLen  = 10;

constexpr int kDefaultWifiPort   = 14560;
constexpr int kDefaultSerialBaud = 500'000;
constexpr int kWifiTimeoutMs     = 2000;
constexpr int kPollIntervalMs    = 3000; // gap between scan rounds

// ── Serial detection ──────────────────────────────────────────────────────────

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

bool probeSerialPort(const std::string& portName, int baud) {
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
    WriteFile(hPort, kSerialProbe.data(),
              static_cast<DWORD>(kSerialProbe.size()), &written, nullptr);

    std::array<char, 6> response{};
    DWORD bytesRead = 0;
    ReadFile(hPort, response.data(),
             static_cast<DWORD>(response.size()), &bytesRead, nullptr);

    CloseHandle(hPort);
    return (bytesRead == kSerialAck.size()) && (response == kSerialAck);
}

// Returns the first COM port name that responds to the NHAWK? handshake, or "".
std::string detectSerial(int baud) {
    for (const auto& port : enumerateComPorts()) {
        if (probeSerialPort(port, baud)) {
            std::cout << "  [serial] Nanohawk found on " << port
                      << " @ " << baud << " baud\n";
            return port;
        }
    }
    return {};
}

// ── WiFi detection ────────────────────────────────────────────────────────────

// Returns the drone IP string on success, or "" if nothing replied.
std::string detectWifi(int port, int timeoutMs) {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return {}; }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) { WSACleanup(); return {}; }

    constexpr int kOn = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
               reinterpret_cast<const char*>(&kOn), sizeof(kOn));

    const DWORD tv = static_cast<DWORD>(timeoutMs);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const char*>(&tv), sizeof(tv));

    sockaddr_in bindAddr{};
    bindAddr.sin_family      = AF_INET;
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_port        = 0;
    bind(sock, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr));

    sockaddr_in destAddr{};
    destAddr.sin_family      = AF_INET;
    destAddr.sin_addr.s_addr = INADDR_BROADCAST;
    destAddr.sin_port        = htons(static_cast<u_short>(port));

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

    char ip[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &senderAddr.sin_addr, ip, sizeof(ip));
    std::cout << "  [wifi]   Nanohawk found at " << ip << ":" << port << "\n";
    return std::string(ip);
}

// ── Launch helpers ────────────────────────────────────────────────────────────

std::filesystem::path executableDir() {
    char buffer[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0) { return std::filesystem::current_path(); }
    return std::filesystem::path(buffer).parent_path();
}

int launchAgentBinary() {
    const auto dir = executableDir();
    const auto gui = dir / "nanohawk_agent.exe";
    const auto cli = dir / "nanohawk_agent_cli.exe";

    std::filesystem::path target;
    if (std::filesystem::exists(gui)) {
        target = gui;
    } else if (std::filesystem::exists(cli)) {
        target = cli;
    }

    if (target.empty()) {
        std::cerr << "no agent executable found beside launcher\n";
        return 1;
    }

    const std::string command = "\"" + target.string() + "\"";
    return std::system(command.c_str());
}

} // namespace

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    std::cout << "nanohawk launcher — scanning for drone\n";
    std::cout << "  WiFi broadcast port : " << kDefaultWifiPort   << "\n";
    std::cout << "  Serial baud rate    : " << kDefaultSerialBaud << " baud\n\n";

    while (true) {
        std::cout << "[scan] checking for Nanohawk...\n";

        // 1. Try WiFi first — a single UDP round-trip is faster than probing
        //    every COM port one by one.
        const std::string wifiIp = detectWifi(kDefaultWifiPort, kWifiTimeoutMs);
        if (!wifiIp.empty()) {
            std::cout << "[launch] drone detected via WiFi at "
                      << wifiIp << " — starting agent\n";
            const int code = launchAgentBinary();
            if (code != 0) {
                std::cerr << "[warn] agent exited with code " << code << "\n";
            }
            // Keep watching: the drone may disconnect and reconnect.
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
            continue;
        }

        // 2. Fall back to USB serial scan.
        const std::string serialPort = detectSerial(kDefaultSerialBaud);
        if (!serialPort.empty()) {
            std::cout << "[launch] drone detected on " << serialPort
                      << " — starting agent\n";
            const int code = launchAgentBinary();
            if (code != 0) {
                std::cerr << "[warn] agent exited with code " << code << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
            continue;
        }

        // 3. Nothing found — pause before the next scan round.
        std::cout << "[scan] no drone detected; retrying in "
                  << kPollIntervalMs / 1000 << " s\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
    }
}
