#pragma once

#include "planning/MissionExecutor.hpp"
#include "planning/MissionPlanner.hpp"
#include "telemetry/VehicleStateStore.hpp"

#include <optional>
#include <string>

namespace nanohawk::app {

class ServiceLocator {
public:
    ServiceLocator(planning::MissionPlanner planner, planning::MissionExecutor executor, telemetry::VehicleStateStore stateStore);

    planning::MissionPlanner& planner();
    planning::MissionExecutor& executor();
    telemetry::VehicleStateStore& stateStore();

    [[nodiscard]] std::optional<planning::MissionPlan> planFromPrompt(const std::string& prompt, std::string& error);
    [[nodiscard]] bool validatePlan(const planning::MissionPlan& plan, std::string& error);
    [[nodiscard]] bool executePlan(const planning::MissionPlan& plan, std::string& error);
    void requestAbort();
    void clearAbort();

private:
    planning::MissionPlanner planner_;
    planning::MissionExecutor executor_;
    telemetry::VehicleStateStore stateStore_;
};

} // namespace nanohawk::app

