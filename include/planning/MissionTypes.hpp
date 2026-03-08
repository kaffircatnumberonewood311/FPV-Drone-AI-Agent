#pragma once

#include <string>
#include <vector>

namespace nanohawk::planning {

enum class ActionType {
    Takeoff,
    MoveBody,
    Hover,
    Land
};

struct MissionAction {
    ActionType type{ActionType::Hover};
    double altitudeM{0.0};
    double forwardM{0.0};
    double rightM{0.0};
    double yawDeg{0.0};
    double durationS{0.0};
};

struct MissionPlan {
    std::string missionName{"unnamed"};
    bool requiresOperatorAuthorize{true};
    double maxAltitudeM{1.0};
    std::vector<MissionAction> actions;
};

struct VehicleSnapshot {
    double batteryPercent{100.0};
    bool linkHealthy{true};
    bool operatorAuthorized{false};
    double xM{0.0};
    double yM{0.0};
    double altitudeM{0.0};
};

} // namespace nanohawk::planning

