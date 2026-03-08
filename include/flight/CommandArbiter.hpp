#pragma once

#include "flight/ArduPilotGuidedAdapter.hpp"
#include "flight/TakeoffLand.hpp"
#include "flight/VelocityController.hpp"
#include "planning/MissionTypes.hpp"

#include <string>

namespace nanohawk::flight {

class CommandArbiter {
public:
    CommandArbiter(ArduPilotGuidedAdapter adapter, VelocityController velocityController, TakeoffLand takeoffLand);

    [[nodiscard]] bool dispatch(planning::MissionAction action, std::string& error);

private:
    ArduPilotGuidedAdapter adapter_;
    VelocityController velocityController_;
    TakeoffLand takeoffLand_;
};

} // namespace nanohawk::flight

