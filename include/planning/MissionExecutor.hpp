#pragma once

#include "flight/CommandArbiter.hpp"
#include "planning/TaskGraph.hpp"
#include "safety/AbortController.hpp"
#include "safety/RuleEngine.hpp"

namespace nanohawk::planning {

class MissionExecutor {
public:
    MissionExecutor(TaskGraph taskGraph, safety::RuleEngine ruleEngine, flight::CommandArbiter commandArbiter, safety::AbortController abortController);

    [[nodiscard]] bool validate(const MissionPlan& plan, const VehicleSnapshot& snapshot, std::string& error) const;
    [[nodiscard]] bool execute(const MissionPlan& plan, const VehicleSnapshot& snapshot, std::string& error);
    void requestAbort();
    void clearAbort();

private:
    TaskGraph taskGraph_;
    safety::RuleEngine ruleEngine_;
    flight::CommandArbiter commandArbiter_;
    safety::AbortController abortController_;
};

} // namespace nanohawk::planning

