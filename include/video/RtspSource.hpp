#pragma once

#include "video/VideoSource.hpp"

#include <string>

namespace nanohawk::video {

class RtspSource final : public VideoSource {
public:
    explicit RtspSource(std::string uri);

    [[nodiscard]] bool open(std::string& error) override;
    [[nodiscard]] std::string name() const override;

private:
    std::string uri_;
};

} // namespace nanohawk::video

