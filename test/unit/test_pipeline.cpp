#include "app/Bootstrap.hpp"

#include <iostream>

int main() {
    nanohawk::app::Bootstrap bootstrap;
    auto services = bootstrap.build("http://127.0.0.1:8080/v1", "udp://0.0.0.0:14550");

    std::string error;
    const auto plan = services.planFromPrompt("Takeoff, move forward, hover, land", error);
    if (!plan.has_value()) {
        std::cerr << "planner failed: " << error << '\n';
        return 1;
    }

    if (!services.validatePlan(*plan, error)) {
        std::cerr << "validation failed: " << error << '\n';
        return 2;
    }

    if (!services.executePlan(*plan, error)) {
        std::cerr << "executor failed: " << error << '\n';
        return 3;
    }

    std::cout << "pipeline test passed\n";
    return 0;
}

