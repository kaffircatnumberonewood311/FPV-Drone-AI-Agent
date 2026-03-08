#include "telemetry/HeartbeatMonitor.hpp"

namespace nanohawk::telemetry {

void HeartbeatMonitor::markHeartbeat() {
    healthy_ = true;
}

bool HeartbeatMonitor::isHealthy() const {
    return healthy_;
}

} // namespace nanohawk::telemetry

