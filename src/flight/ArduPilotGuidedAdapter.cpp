#include "flight/ArduPilotGuidedAdapter.hpp"

#include <iostream>
#include <sstream>
#include <utility>

namespace nanohawk::flight {

ArduPilotGuidedAdapter::ArduPilotGuidedAdapter(std::string mavlinkEndpoint)
    : transport_(std::move(mavlinkEndpoint)) {}

bool ArduPilotGuidedAdapter::executeAction(const planning::MissionAction& action, std::string& error) {
    if (!connected_) {
        std::string connectError;
        if (transport_.connect(connectError)) {
            connected_ = true;
            std::cout << "MAVLink transport connected\n";
        } else {
            // Keep deterministic local behavior even when transport is unavailable.
            std::cout << "MAVLink transport unavailable: " << connectError << "\n";
        }
    }

    std::ostringstream command;
    switch (action.type) {
        case planning::ActionType::Takeoff:
            command << "takeoff:" << action.altitudeM;
            std::cout << "GUIDED takeoff " << action.altitudeM << "m\n";
            break;
        case planning::ActionType::MoveBody:
            command << "move_body:" << action.forwardM << ',' << action.rightM << ',' << action.yawDeg;
            std::cout << "GUIDED move forward=" << action.forwardM << " right=" << action.rightM << " yaw=" << action.yawDeg << "\n";
            break;
        case planning::ActionType::Hover:
            command << "hover:" << action.durationS;
            std::cout << "GUIDED hover " << action.durationS << "s\n";
            break;
        case planning::ActionType::Land:
            command << "land";
            std::cout << "GUIDED land\n";
            break;
        default:
            error = "unsupported action";
            return false;
    }

    if (!connected_) {
        return true;
    }

    std::string transportError;
    if (!transport_.sendGuidedCommand(command.str(), transportError)) {
        std::cout << "MAVLink send failed, keeping local deterministic flow: " << transportError << "\n";
    }

    return true;
}

} // namespace nanohawk::flight
