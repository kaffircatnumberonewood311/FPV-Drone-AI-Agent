#pragma once

namespace nanohawk::safety {

class LinkLossGuard {
public:
    [[nodiscard]] bool isSafe(bool linkHealthy) const;
};

} // namespace nanohawk::safety

