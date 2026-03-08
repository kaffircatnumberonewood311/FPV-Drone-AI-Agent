#pragma once

namespace nanohawk::safety {

class BatteryGuard {
public:
    explicit BatteryGuard(double minBatteryPercent);

    [[nodiscard]] bool isSafe(double batteryPercent) const;

private:
    double minBatteryPercent_;
};

} // namespace nanohawk::safety

