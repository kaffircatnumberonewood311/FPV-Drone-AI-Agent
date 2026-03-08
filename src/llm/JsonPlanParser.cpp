#include "llm/JsonPlanParser.hpp"

#include <regex>
#include <string>

namespace nanohawk::llm {

namespace {

std::optional<double> readNumber(const std::string& json, const std::string& key) {
    const std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return std::stod(match[1].str());
    }
    return std::nullopt;
}

bool readBool(const std::string& json, const std::string& key, bool defaultValue) {
    const std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1].str() == "true";
    }
    return defaultValue;
}

std::string readString(const std::string& json, const std::string& key, const std::string& defaultValue) {
    const std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1].str();
    }
    return defaultValue;
}

std::string readActionType(const std::string& json, size_t from, size_t& tokenPos) {
    constexpr const char* token = "\"type\":\"";
    tokenPos = json.find(token, from);
    if (tokenPos == std::string::npos) {
        return {};
    }

    const size_t valueStart = tokenPos + std::char_traits<char>::length(token);
    const size_t valueEnd = json.find('"', valueStart);
    if (valueEnd == std::string::npos) {
        return {};
    }

    return json.substr(valueStart, valueEnd - valueStart);
}

} // namespace

std::optional<planning::MissionPlan> JsonPlanParser::parse(const std::string& missionJson, std::string& error) const {
    planning::MissionPlan plan;
    plan.missionName = readString(missionJson, "mission_name", "unnamed");
    plan.requiresOperatorAuthorize = readBool(missionJson, "requires_operator_authorize", true);

    const auto maxAltitude = readNumber(missionJson, "max_altitude_m");
    if (!maxAltitude.has_value()) {
        error = "missing max_altitude_m";
        return std::nullopt;
    }
    plan.maxAltitudeM = *maxAltitude;

    size_t cursor = 0;
    while (cursor < missionJson.size()) {
        size_t typePos = std::string::npos;
        const std::string type = readActionType(missionJson, cursor, typePos);
        if (type.empty() || typePos == std::string::npos) {
            break;
        }

        const size_t objectStart = missionJson.rfind('{', typePos);
        const size_t objectEnd = missionJson.find('}', typePos);
        const std::string objectSlice = (objectStart != std::string::npos && objectEnd != std::string::npos && objectEnd > objectStart)
            ? missionJson.substr(objectStart, objectEnd - objectStart + 1)
            : missionJson.substr(typePos);

        planning::MissionAction action;
        if (type == "takeoff") {
            action.type = planning::ActionType::Takeoff;
            action.altitudeM = readNumber(objectSlice, "altitude_m").value_or(1.0);
            plan.actions.push_back(action);
        } else if (type == "move_body") {
            action.type = planning::ActionType::MoveBody;
            action.forwardM = readNumber(objectSlice, "forward_m").value_or(0.0);
            action.rightM = readNumber(objectSlice, "right_m").value_or(0.0);
            action.yawDeg = readNumber(objectSlice, "yaw_deg").value_or(0.0);
            plan.actions.push_back(action);
        } else if (type == "hover") {
            action.type = planning::ActionType::Hover;
            action.durationS = readNumber(objectSlice, "duration_s").value_or(1.0);
            plan.actions.push_back(action);
        } else if (type == "land") {
            action.type = planning::ActionType::Land;
            plan.actions.push_back(action);
        }

        cursor = (objectEnd == std::string::npos) ? missionJson.size() : objectEnd + 1;
    }

    if (plan.actions.empty()) {
        error = "no mission actions parsed";
        return std::nullopt;
    }

    return plan;
}

} // namespace nanohawk::llm

