#include "safety/RuleEngine.hpp"

#include <cmath>

namespace nanohawk::safety {

RuleEngine::RuleEngine()
    : geofence_(-5.0, 5.0, -5.0, 5.0), batteryGuard_(30.0), maxAltitudeM_(1.2) {}

bool RuleEngine::validate(const planning::MissionPlan& plan, const planning::VehicleSnapshot& snapshot, std::string& error) const {
    if (!batteryGuard_.isSafe(snapshot.batteryPercent)) {
        error = "battery too low";
        return false;
    }

    if (!linkLossGuard_.isSafe(snapshot.linkHealthy)) {
        error = "link is unhealthy";
        return false;
    }

    if (plan.requiresOperatorAuthorize && !snapshot.operatorAuthorized) {
        error = "operator authorization missing";
        return false;
    }

    if (!geofence_.contains(snapshot.xM, snapshot.yM)) {
        error = "vehicle outside geofence";
        return false;
    }

    if (plan.maxAltitudeM > maxAltitudeM_) {
        error = "plan exceeds maximum allowed altitude";
        return false;
    }

    for (const auto& action : plan.actions) {
        if (action.type == planning::ActionType::Takeoff && action.altitudeM > maxAltitudeM_) {
            error = "takeoff altitude violates limit";
            return false;
        }
        if (action.type == planning::ActionType::MoveBody) {
            const double distance = std::sqrt(action.forwardM * action.forwardM + action.rightM * action.rightM);
            if (distance > 3.0) {
                error = "single move action exceeds indoor distance budget";
                return false;
            }
        }
    }

    return true;
}

} // namespace nanohawk::safety

