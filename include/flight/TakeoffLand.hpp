#pragma once

#include "planning/MissionTypes.hpp"

namespace nanohawk::flight {

class TakeoffLand {
public:
    [[nodiscard]] bool isTakeoffOrLand(const planning::MissionAction& action) const;
};

} // namespace nanohawk::flight

