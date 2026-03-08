#include "flight/TakeoffLand.hpp"

namespace nanohawk::flight {

bool TakeoffLand::isTakeoffOrLand(const planning::MissionAction& action) const {
    return action.type == planning::ActionType::Takeoff || action.type == planning::ActionType::Land;
}

} // namespace nanohawk::flight

