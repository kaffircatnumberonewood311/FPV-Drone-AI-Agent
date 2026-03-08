#pragma once

#include "planning/MissionTypes.hpp"

#include <vector>

namespace nanohawk::planning {

class TaskGraph {
public:
    [[nodiscard]] std::vector<MissionAction> linearize(const MissionPlan& plan) const;
};

} // namespace nanohawk::planning

