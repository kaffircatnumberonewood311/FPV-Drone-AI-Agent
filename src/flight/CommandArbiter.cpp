#include "flight/CommandArbiter.hpp"

namespace nanohawk::flight {

CommandArbiter::CommandArbiter(ArduPilotGuidedAdapter adapter, VelocityController velocityController, TakeoffLand takeoffLand)
    : adapter_(std::move(adapter)), velocityController_(std::move(velocityController)), takeoffLand_(std::move(takeoffLand)) {}

bool CommandArbiter::dispatch(planning::MissionAction action, std::string& error) {
    if (!takeoffLand_.isTakeoffOrLand(action)) {
        velocityController_.clamp(action, 1.0);
    }
    return adapter_.executeAction(action, error);
}

} // namespace nanohawk::flight

