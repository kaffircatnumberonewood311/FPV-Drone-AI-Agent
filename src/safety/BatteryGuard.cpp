#include "safety/BatteryGuard.hpp"

namespace nanohawk::safety {

BatteryGuard::BatteryGuard(double minBatteryPercent) : minBatteryPercent_(minBatteryPercent) {}

bool BatteryGuard::isSafe(double batteryPercent) const {
    return batteryPercent >= minBatteryPercent_;
}

} // namespace nanohawk::safety

