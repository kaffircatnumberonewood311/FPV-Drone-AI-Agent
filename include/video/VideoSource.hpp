#pragma once

#include <string>

namespace nanohawk::video {

class VideoSource {
public:
    virtual ~VideoSource() = default;
    [[nodiscard]] virtual bool open(std::string& error) = 0;
    [[nodiscard]] virtual std::string name() const = 0;
};

} // namespace nanohawk::video

