#pragma once

#include <mutex>
#include <string>

namespace nanohawk::video {

class FrameBus {
public:
    void publishSource(const std::string& source);
    [[nodiscard]] std::string latestSource() const;

private:
    mutable std::mutex mutex_;
    std::string source_;
};

} // namespace nanohawk::video

