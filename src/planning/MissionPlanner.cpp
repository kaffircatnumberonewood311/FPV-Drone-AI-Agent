#include "planning/MissionPlanner.hpp"

namespace nanohawk::planning {

MissionPlanner::MissionPlanner(llm::PromptCompiler compiler, llm::LlmClient llmClient, llm::JsonPlanParser parser)
    : compiler_(std::move(compiler)), llmClient_(std::move(llmClient)), parser_(std::move(parser)) {}

std::optional<MissionPlan> MissionPlanner::buildPlan(const std::string& prompt, std::string& error) const {
    const std::string compiled = compiler_.compile(prompt);
    const std::string missionJson = llmClient_.requestMissionJson(compiled);
    return parser_.parse(missionJson, error);
}

} // namespace nanohawk::planning

