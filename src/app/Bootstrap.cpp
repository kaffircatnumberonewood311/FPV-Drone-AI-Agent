#include "app/Bootstrap.hpp"

#include "app/DeviceWatcher.hpp"
#include "flight/ArduPilotGuidedAdapter.hpp"
#include "flight/CommandArbiter.hpp"
#include "flight/TakeoffLand.hpp"
#include "flight/VelocityController.hpp"
#include "llm/JsonPlanParser.hpp"
#include "llm/LlmClient.hpp"
#include "llm/PromptCompiler.hpp"
#include "planning/MissionExecutor.hpp"
#include "planning/MissionPlanner.hpp"
#include "planning/TaskGraph.hpp"
#include "safety/AbortController.hpp"
#include "safety/RuleEngine.hpp"

#include <array>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

namespace nanohawk::app {

namespace {

struct EndpointConfig {
    std::string llmBaseUrl{"http://127.0.0.1:8080/v1"};
    std::string mavlinkEndpoint{"udp://0.0.0.0:14550"};
};

std::string readTextFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return {};
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string loadEndpointsYaml(const std::string& requestedPath) {
    const std::array<std::string, 4> candidates = {
        requestedPath,
        "config/endpoints.yaml",
        "../config/endpoints.yaml",
        "../../config/endpoints.yaml"
    };

    for (const auto& candidate : candidates) {
        const std::string content = readTextFile(candidate);
        if (!content.empty()) {
            return content;
        }
    }

    return {};
}

std::string extractYamlValue(const std::string& yaml, const std::string& section, const std::string& key) {
    const std::regex pattern("(?:^|\\n)" + section + "\\s*:\\s*(?:\\n|\\r\\n)(?:[ \\t]+.*(?:\\n|\\r\\n))*?[ \\t]+" + key + "\\s*:\\s*([^\\r\\n#]+)");
    std::smatch match;
    if (!std::regex_search(yaml, match, pattern)) {
        return {};
    }

    std::string value = match[1].str();
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
        value.pop_back();
    }
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.erase(value.begin());
    }
    return value;
}

EndpointConfig loadEndpointConfig(const std::string& endpointsConfigPath) {
    EndpointConfig config;
    const std::string yaml = loadEndpointsYaml(endpointsConfigPath);
    if (yaml.empty()) {
        return config;
    }

    const std::string llmBaseUrl = extractYamlValue(yaml, "llm", "base_url");
    if (!llmBaseUrl.empty()) {
        config.llmBaseUrl = llmBaseUrl;
    }

    const std::string mavlinkUdp = extractYamlValue(yaml, "mavlink", "udp_listen");
    if (!mavlinkUdp.empty()) {
        config.mavlinkEndpoint = mavlinkUdp;
    } else {
        const std::string serialPort = extractYamlValue(yaml, "mavlink", "serial_port");
        const std::string serialBaud = extractYamlValue(yaml, "mavlink", "serial_baud");

        if (!serialPort.empty()) {
            if (!serialBaud.empty()) {
                config.mavlinkEndpoint = "serial://" + serialPort + ":" + serialBaud;
            } else {
                config.mavlinkEndpoint = "serial://" + serialPort + ":57600";
            }
        }
    }

    return config;
}

} // namespace

ServiceLocator Bootstrap::build(const std::string& llmEndpoint, const std::string& mavlinkEndpoint) const {
    planning::MissionPlanner planner(
        llm::PromptCompiler{},
        llm::LlmClient{llmEndpoint},
        llm::JsonPlanParser{}
    );

    planning::MissionExecutor executor(
        planning::TaskGraph{},
        safety::RuleEngine{},
        flight::CommandArbiter{flight::ArduPilotGuidedAdapter{mavlinkEndpoint}, flight::VelocityController{}, flight::TakeoffLand{}},
        safety::AbortController{}
    );

    telemetry::VehicleStateStore stateStore;
    planning::VehicleSnapshot defaultState;
    defaultState.batteryPercent = 95.0;
    defaultState.linkHealthy = true;
    defaultState.operatorAuthorized = true;
    stateStore.update(defaultState);

    return ServiceLocator(std::move(planner), std::move(executor), std::move(stateStore));
}

ServiceLocator Bootstrap::buildFromConfig(const std::string& endpointsConfigPath) const {
    const EndpointConfig config = loadEndpointConfig(endpointsConfigPath);
    return build(config.llmBaseUrl, config.mavlinkEndpoint);
}

ServiceLocator Bootstrap::buildAutoDetect(const std::string& endpointsConfigPath,
                                          int                discoveryPort,
                                          int                serialBaud,
                                          int                timeoutMs) const {
    // Load base config for the LLM URL and any hard-coded fallback endpoint.
    const EndpointConfig baseConfig = loadEndpointConfig(endpointsConfigPath);

    // Attempt physical drone detection.
    const DeviceWatcher watcher;
    const DetectedDevice device = watcher.detectAny(discoveryPort, serialBaud, timeoutMs);

    // Prefer the auto-detected endpoint; fall back to whatever is in config.
    const std::string mavlinkEndpoint = device.found()
                                      ? device.endpoint
                                      : baseConfig.mavlinkEndpoint;

    return build(baseConfig.llmBaseUrl, mavlinkEndpoint);
}

} // namespace nanohawk::app

