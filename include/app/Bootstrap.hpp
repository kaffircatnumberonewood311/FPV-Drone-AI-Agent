#pragma once

#include "app/ServiceLocator.hpp"

#include <string>

namespace nanohawk::app {

class Bootstrap {
public:
    // Build from explicit endpoints (used by tests and CLI overrides).
    [[nodiscard]] ServiceLocator build(const std::string& llmEndpoint,
                                       const std::string& mavlinkEndpoint) const;

    // Build by reading endpoints.yaml; falls back to compiled-in defaults when
    // the file is missing or a key is absent.
    [[nodiscard]] ServiceLocator buildFromConfig(
        const std::string& endpointsConfigPath = "config/endpoints.yaml") const;

    // Build by auto-detecting the drone:
    //   1. Send a WiFi UDP broadcast on <discoveryPort> and wait <timeoutMs>.
    //   2. If no WiFi reply, enumerate COM ports and probe each with the
    //      NHAWK? serial handshake at <serialBaud>.
    //   3. Use the detected endpoint; fall back to endpoints.yaml if nothing found.
    [[nodiscard]] ServiceLocator buildAutoDetect(
        const std::string& endpointsConfigPath = "config/endpoints.yaml",
        int                discoveryPort        = 14560,
        int                serialBaud           = 500'000,
        int                timeoutMs            = 2000) const;
};

} // namespace nanohawk::app

