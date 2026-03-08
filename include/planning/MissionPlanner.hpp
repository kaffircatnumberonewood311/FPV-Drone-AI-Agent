#pragma once

#include "llm/JsonPlanParser.hpp"
#include "llm/LlmClient.hpp"
#include "llm/PromptCompiler.hpp"
#include "planning/MissionTypes.hpp"

#include <optional>
#include <string>

namespace nanohawk::planning {

class MissionPlanner {
public:
    MissionPlanner(llm::PromptCompiler compiler, llm::LlmClient llmClient, llm::JsonPlanParser parser);

    [[nodiscard]] std::optional<MissionPlan> buildPlan(const std::string& prompt, std::string& error) const;

private:
    llm::PromptCompiler compiler_;
    llm::LlmClient llmClient_;
    llm::JsonPlanParser parser_;
};

} // namespace nanohawk::planning

