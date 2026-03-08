#include "safety/LinkLossGuard.hpp"

namespace nanohawk::safety {

bool LinkLossGuard::isSafe(bool linkHealthy) const {
    return linkHealthy;
}

} // namespace nanohawk::safety

