#include "app/DeviceWatcher.hpp"
#include "msp/MspClient.hpp"

#include <iostream>
#include <iomanip>

int main() {
    nanohawk::app::DeviceWatcher watcher;

    std::cout << "=== Nanohawk Device Detection ===\n\n";

    // ── WiFi broadcast discovery ──────────────────────────────────────────────
    std::cout << "[1/2] WiFi UDP broadcast (1 s timeout)...\n";
    const auto wifi = watcher.detectWifi(14560, 1000);
    if (wifi.found()) {
        std::cout << "  FOUND  " << wifi.description << "\n";
        std::cout << "  Endpoint: " << wifi.endpoint << "\n";
    } else {
        std::cout << "  No WiFi response (expected if no WiFi companion MCU).\n";
    }

    // ── USB serial scan ───────────────────────────────────────────────────────
    std::cout << "\n[2/2] USB serial scan (VID/PID registry + handshake)...\n";
    const auto serial = watcher.detectSerial(115200);
    if (serial.found()) {
        std::cout << "  FOUND  " << serial.description << "\n";
        std::cout << "  Endpoint: " << serial.endpoint << "\n";
    } else {
        std::cout << "  No serial drone device found.\n";
    }

    // ── Best available endpoint ───────────────────────────────────────────────
    std::cout << "\n[auto] detectAny() — WiFi first, USB fallback...\n";
    const auto any = watcher.detectAny(14560, 115200, 1000);
    if (!any.found()) {
        std::cerr << "  No drone detected on any interface.\n";
        return 1;
    }

    std::cout << "  CONNECTED  " << any.description << "\n";
    std::cout << "  Endpoint:  " << any.endpoint << "\n";

    // ── MSP session ───────────────────────────────────────────────────────────
    if (any.transport != nanohawk::app::DetectedDevice::Transport::Serial) {
        std::cout << "\nSkipping MSP session (WiFi endpoint; MSP needs serial).\n";
        return 0;
    }

    std::cout << "\n=== MSP Session on " << any.portOrHost << " ===\n\n";

    nanohawk::msp::MspClient fc(any.portOrHost, any.baudOrPort);
    std::string err;

    if (!fc.connect(err)) {
        std::cerr << "  connect() failed: " << err << "\n";
        return 1;
    }
    std::cout << "  Port open OK.\n";

    // ── FC identification ─────────────────────────────────────────────────────
    std::string variant;
    if (fc.identify(variant, err)) {
        std::cout << "  FC variant : " << variant << "\n";
    } else {
        std::cerr << "  identify() failed: " << err << "\n";
    }

    uint8_t maj = 0, min = 0, pat = 0;
    if (fc.version(maj, min, pat, err)) {
        std::cout << "  FW version : "
                  << static_cast<int>(maj) << "."
                  << static_cast<int>(min) << "."
                  << static_cast<int>(pat) << "\n";
    } else {
        std::cerr << "  version() failed: " << err << "\n";
    }

    // ── Telemetry ─────────────────────────────────────────────────────────────
    nanohawk::msp::Analog batt;
    if (fc.readAnalog(batt, err)) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Battery    : " << batt.batteryVolts << " V"
                  << "  (" << batt.mAhDrawn << " mAh drawn,"
                  << "  RSSI " << batt.rssi << "/1023,"
                  << "  " << batt.amperage << " A)\n";
    } else {
        std::cerr << "  readAnalog() failed: " << err << "\n";
    }

    nanohawk::msp::Attitude att;
    if (fc.readAttitude(att, err)) {
        std::cout << "  Attitude   : "
                  << "roll "  << att.rollDeg  << " deg, "
                  << "pitch " << att.pitchDeg << " deg, "
                  << "yaw "   << att.yawDeg   << " deg\n";
    } else {
        std::cerr << "  readAttitude() failed: " << err << "\n";
    }

    nanohawk::msp::RcChannels rc;
    if (fc.readRc(rc, err)) {
        std::cout << "  RC channels: ";
        for (int i = 0; i < 8; ++i) {
            std::cout << rc.ch[i];
            if (i < 7) { std::cout << "  "; }
        }
        std::cout << "  µs\n";
        std::cout << "             (roll  pitch  thr    yaw    arm    aux2   aux3   aux4)\n";
    } else {
        std::cerr << "  readRc() failed: " << err << "\n";
    }

    std::cout << "\nDrone detected and MSP telemetry verified. Agent ready.\n";
    return 0;
}
