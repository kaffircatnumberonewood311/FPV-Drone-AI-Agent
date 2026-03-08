#include "app/ServiceLocator.hpp"

namespace nanohawk::app {

ServiceLocator::ServiceLocator(planning::MissionPlanner planner, planning::MissionExecutor executor, telemetry::VehicleStateStore stateStore)
    : planner_(std::move(planner)), executor_(std::move(executor)), stateStore_(std::move(stateStore)) {}

planning::MissionPlanner& ServiceLocator::planner() {
    return planner_;
}

planning::MissionExecutor& ServiceLocator::executor() {
    return executor_;
}

telemetry::VehicleStateStore& ServiceLocator::stateStore() {
    return stateStore_;
}

std::optional<planning::MissionPlan> ServiceLocator::planFromPrompt(const std::string& prompt, std::string& error) {
    return planner_.buildPlan(prompt, error);
}

bool ServiceLocator::validatePlan(const planning::MissionPlan& plan, std::string& error) {
    return executor_.validate(plan, stateStore_.current(), error);
}

bool ServiceLocator::executePlan(const planning::MissionPlan& plan, std::string& error) {
    executor_.clearAbort();
    return executor_.execute(plan, stateStore_.current(), error);
}

void ServiceLocator::requestAbort() {
    executor_.requestAbort();
}

void ServiceLocator::clearAbort() {
    executor_.clearAbort();
}

} // namespace nanohawk::app

