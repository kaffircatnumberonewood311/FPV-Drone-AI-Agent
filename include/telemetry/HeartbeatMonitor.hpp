#pragma once

namespace nanohawk::telemetry {

class HeartbeatMonitor {
public:
    void markHeartbeat();
    [[nodiscard]] bool isHealthy() const;

private:
    bool healthy_{false};
};

} // namespace nanohawk::telemetry

