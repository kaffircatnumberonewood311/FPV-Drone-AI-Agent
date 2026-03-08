#pragma once

#include <string>

namespace nanohawk::app {

// Describes a detected Nanohawk drone endpoint.
struct DetectedDevice {
    enum class Transport { None, Serial, WiFi };

    Transport   transport{Transport::None};
    std::string portOrHost;  // "COM5"  or  "192.168.4.1"
    int         baudOrPort{0};  // 500000 (serial baud) or 14560 (UDP port)
    std::string endpoint;    // "serial://COM5:500000"  or  "udp://192.168.4.1:14560"
    std::string description; // human-readable label shown in GUI/logs

    [[nodiscard]] bool found() const { return transport != Transport::None; }
};

// Scans for the Nanohawk companion MCU via USB serial and/or WiFi UDP broadcast.
//
// USB serial path:
//   1. Enumerate all COM ports from the Windows registry.
//   2. Open each port at the configured baud rate.
//   3. Send the 6-byte probe frame "NHAWK?" and wait up to 200 ms for "NHAWK!".
//   4. First port that responds is returned.
//
// WiFi path:
//   1. Send UDP broadcast "NHAWK_DISCOVER" to 255.255.255.255:<discoveryPort>.
//   2. Wait up to <timeoutMs> for a "NHAWK_FOUND" reply from the companion MCU.
//   3. Sender IP + port are returned as a udp:// endpoint.
//
// The companion MCU firmware must implement both sides of these protocols.
// See firmware/companion_mcu/protocol.h for the canonical frame definitions.
class DeviceWatcher {
public:
    // Probe every available COM port for the Nanohawk companion MCU.
    // Returns the first port that responds to the NHAWK? handshake.
    [[nodiscard]] DetectedDevice detectSerial(int baud = 500'000) const;

    // Send a UDP broadcast discovery packet and wait for the companion MCU reply.
    [[nodiscard]] DetectedDevice detectWifi(int discoveryPort = 14560,
                                            int timeoutMs     = 2000) const;

    // Try WiFi first (single round-trip, faster than probing all COM ports),
    // then fall back to USB serial scan. Returns the first found device.
    [[nodiscard]] DetectedDevice detectAny(int discoveryPort = 14560,
                                           int serialBaud    = 500'000,
                                           int timeoutMs     = 2000) const;

    // Legacy boolean API kept for callers that only need a yes/no answer.
    [[nodiscard]] bool detectVehicleEndpoint() const;
    [[nodiscard]] bool detectVideoEndpoint() const;
};

} // namespace nanohawk::app
