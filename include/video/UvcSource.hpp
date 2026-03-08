#pragma once

#include "video/VideoSource.hpp"

#include <string>

namespace nanohawk::video {

class UvcSource final : public VideoSource {
public:
    explicit UvcSource(int cameraIndex);

    [[nodiscard]] bool open(std::string& error) override;
    [[nodiscard]] std::string name() const override;

private:
    int cameraIndex_;
};

} // namespace nanohawk::video

