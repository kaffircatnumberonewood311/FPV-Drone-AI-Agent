#include "video/FrameBus.hpp"

namespace nanohawk::video {

void FrameBus::publishSource(const std::string& source) {
    std::lock_guard<std::mutex> lock(mutex_);
    source_ = source;
}

std::string FrameBus::latestSource() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return source_;
}

} // namespace nanohawk::video

