#include "planning/MissionExecutor.hpp"

namespace nanohawk::planning {

MissionExecutor::MissionExecutor(TaskGraph taskGraph, safety::RuleEngine ruleEngine, flight::CommandArbiter commandArbiter, safety::AbortController abortController)
    : taskGraph_(std::move(taskGraph)), ruleEngine_(std::move(ruleEngine)), commandArbiter_(std::move(commandArbiter)), abortController_(std::move(abortController)) {}

bool MissionExecutor::validate(const MissionPlan& plan, const VehicleSnapshot& snapshot, std::string& error) const {
    return ruleEngine_.validate(plan, snapshot, error);
}

bool MissionExecutor::execute(const MissionPlan& plan, const VehicleSnapshot& snapshot, std::string& error) {
    if (!validate(plan, snapshot, error)) {
        return false;
    }

    const auto tasks = taskGraph_.linearize(plan);
    for (const auto& action : tasks) {
        if (abortController_.isAbortRequested()) {
            error = "mission aborted by operator";
            return false;
        }
        if (!commandArbiter_.dispatch(action, error)) {
            return false;
        }
    }

    return true;
}

void MissionExecutor::requestAbort() {
    abortController_.requestAbort();
}

void MissionExecutor::clearAbort() {
    abortController_.clear();
}

} // namespace nanohawk::planning

