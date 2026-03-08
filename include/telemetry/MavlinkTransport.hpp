#pragma once

#include <memory>
#include <string>

namespace nanohawk::telemetry {

class MavlinkTransport {
public:
    explicit MavlinkTransport(std::string endpoint = "udp://0.0.0.0:14550");
    ~MavlinkTransport();

    MavlinkTransport(MavlinkTransport&&) noexcept;
    MavlinkTransport& operator=(MavlinkTransport&&) noexcept;

    MavlinkTransport(const MavlinkTransport&) = delete;
    MavlinkTransport& operator=(const MavlinkTransport&) = delete;

    [[nodiscard]] bool connect(std::string& error);
    [[nodiscard]] bool sendGuidedCommand(const std::string& command, std::string& error) const;

private:
    struct Impl;

    std::string endpoint_;
    std::unique_ptr<Impl> impl_;
};

} // namespace nanohawk::telemetry

