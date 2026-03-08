#include "flight/VelocityController.hpp"

#include <algorithm>
#include <cmath>

namespace nanohawk::flight {

void VelocityController::clamp(planning::MissionAction& action, double maxVelocityMps) const {
    if (action.type != planning::ActionType::MoveBody) {
        return;
    }

    const double distance = std::sqrt(action.forwardM * action.forwardM + action.rightM * action.rightM);
    if (distance <= maxVelocityMps || distance == 0.0) {
        return;
    }

    const double scale = maxVelocityMps / distance;
    action.forwardM *= scale;
    action.rightM *= scale;
}

} // namespace nanohawk::flight

