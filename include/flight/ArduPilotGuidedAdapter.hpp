#pragma once

#include "planning/MissionTypes.hpp"
#include "telemetry/MavlinkTransport.hpp"

#include <string>

namespace nanohawk::flight {

class ArduPilotGuidedAdapter {
public:
    explicit ArduPilotGuidedAdapter(std::string mavlinkEndpoint = "udp://0.0.0.0:14550");

    [[nodiscard]] bool executeAction(const planning::MissionAction& action, std::string& error);

private:
    mutable telemetry::MavlinkTransport transport_;
    mutable bool connected_{false};
};

} // namespace nanohawk::flight

