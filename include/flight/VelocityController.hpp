#pragma once

#include "planning/MissionTypes.hpp"

namespace nanohawk::flight {

class VelocityController {
public:
    void clamp(planning::MissionAction& action, double maxVelocityMps) const;
};

} // namespace nanohawk::flight

