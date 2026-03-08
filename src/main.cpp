#include "app/Bootstrap.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    nanohawk::app::Bootstrap bootstrap;
    auto services = bootstrap.buildFromConfig();

    std::cout << "=== NanoHawk Agent CLI ===\n";
    std::cout << "Type mission prompts to execute (or 'quit' to exit)\n\n";

    bool interactive = (argc == 1);

    if (!interactive) {
        const std::string prompt = argv[1];
        std::cout << "> " << prompt << '\n';

        std::string error;
        const auto plan = services.planFromPrompt(prompt, error);
        if (!plan.has_value()) {
            std::cerr << "Plan generation failed: " << error << '\n';
            return 1;
        }

        std::cout << "Parsed actions: " << plan->actions.size() << '\n';

        if (!services.validatePlan(*plan, error)) {
            std::cerr << "Safety validation failed: " << error << '\n';
            return 2;
        }

        if (!services.executePlan(*plan, error)) {
            std::cerr << "Mission execution failed: " << error << '\n';
            return 3;
        }

        std::cout << "Mission completed: " << plan->missionName << '\n';
        return 0;
    }

    while (true) {
        std::cout << "> ";
        std::string prompt;
        if (!std::getline(std::cin, prompt)) {
            break;
        }

        if (prompt.empty()) {
            continue;
        }

        if (prompt == "quit" || prompt == "exit" || prompt == "q") {
            std::cout << "Shutting down...\n";
            break;
        }

        std::string error;
        const auto plan = services.planFromPrompt(prompt, error);
        if (!plan.has_value()) {
            std::cerr << "Plan generation failed: " << error << '\n';
            continue;
        }

        std::cout << "Parsed " << plan->actions.size() << " actions for mission: " << plan->missionName << '\n';

        if (!services.validatePlan(*plan, error)) {
            std::cerr << "Safety validation failed: " << error << '\n';
            continue;
        }

        if (!services.executePlan(*plan, error)) {
            std::cerr << "Mission execution failed: " << error << '\n';
            continue;
        }

        std::cout << "Mission completed successfully.\n\n";
    }

    return 0;
}
