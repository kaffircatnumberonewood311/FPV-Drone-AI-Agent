#include "safety/AbortController.hpp"

namespace nanohawk::safety {

void AbortController::requestAbort() {
    requested_ = true;
}

void AbortController::clear() {
    requested_ = false;
}

bool AbortController::isAbortRequested() const {
    return requested_;
}

} // namespace nanohawk::safety

