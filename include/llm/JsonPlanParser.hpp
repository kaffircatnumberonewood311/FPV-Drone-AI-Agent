#pragma once

#include "planning/MissionTypes.hpp"

#include <optional>
#include <string>

namespace nanohawk::llm {

class JsonPlanParser {
public:
    [[nodiscard]] std::optional<planning::MissionPlan> parse(const std::string& missionJson, std::string& error) const;
};

} // namespace nanohawk::llm

