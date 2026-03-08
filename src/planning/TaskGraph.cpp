#include "planning/TaskGraph.hpp"

namespace nanohawk::planning {

std::vector<MissionAction> TaskGraph::linearize(const MissionPlan& plan) const {
    return plan.actions;
}

} // namespace nanohawk::planning

