#include "llm/LlmClient.hpp"
#include "telemetry/MavlinkTransport.hpp"
#include "video/RtspSource.hpp"
#include "video/UvcSource.hpp"

#include <iostream>

int main() {
    std::string error;

    nanohawk::telemetry::MavlinkTransport emptyTransport("");
    if (emptyTransport.connect(error)) {
        std::cerr << "expected empty endpoint connect failure\n";
        return 1;
    }

    nanohawk::video::RtspSource emptyRtsp("");
    error.clear();
    if (emptyRtsp.open(error)) {
        std::cerr << "expected empty RTSP open failure\n";
        return 2;
    }

    nanohawk::video::UvcSource invalidCamera(-1);
    error.clear();
    if (invalidCamera.open(error)) {
        std::cerr << "expected invalid UVC open failure\n";
        return 3;
    }

    nanohawk::llm::LlmClient llm("http://127.0.0.1:8080/v1");
    const std::string mission = llm.requestMissionJson("Takeoff, move forward, hover, land");
    if (mission.find("\"actions\"") == std::string::npos) {
        std::cerr << "expected mission JSON actions\n";
        return 4;
    }

    std::cout << "transport tests passed\n";
    return 0;
}

