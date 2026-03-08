#pragma once

#include "planning/MissionTypes.hpp"
#include "safety/BatteryGuard.hpp"
#include "safety/Geofence.hpp"
#include "safety/LinkLossGuard.hpp"

#include <string>

namespace nanohawk::safety {

class RuleEngine {
public:
    RuleEngine();

    [[nodiscard]] bool validate(const planning::MissionPlan& plan, const planning::VehicleSnapshot& snapshot, std::string& error) const;

private:
    Geofence geofence_;
    BatteryGuard batteryGuard_;
    LinkLossGuard linkLossGuard_;
    double maxAltitudeM_;
};

} // namespace nanohawk::safety

