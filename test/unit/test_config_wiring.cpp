#include "app/Bootstrap.hpp"

#include <iostream>

int main() {
    nanohawk::app::Bootstrap bootstrap;

    std::cout << "Testing UDP endpoint config (default endpoints.yaml):\n";
    auto services1 = bootstrap.buildFromConfig();
    std::cout << "  Bootstrap succeeded with default config\n";

    std::cout << "\nTesting serial endpoint config fallback:\n";
    auto services2 = bootstrap.buildFromConfig("../../config/endpoints_serial.yaml");
    std::cout << "  Bootstrap succeeded with serial fallback config\n";

    std::cout << "\nTesting explicit endpoint override:\n";
    auto services3 = bootstrap.build("http://192.168.1.100:8080/v1", "serial://COM3:115200");
    std::cout << "  Bootstrap succeeded with explicit endpoints\n";

    std::cout << "\nendpoint config wiring test passed\n";
    return 0;
}
