#include "video/UvcSource.hpp"

#ifdef NANOHAWK_WITH_OPENCV
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#endif

namespace nanohawk::video {

UvcSource::UvcSource(int cameraIndex) : cameraIndex_(cameraIndex) {}

bool UvcSource::open(std::string& error) {
    if (cameraIndex_ < 0) {
        error = "invalid UVC camera index";
        return false;
    }

#ifdef NANOHAWK_WITH_OPENCV
    cv::VideoCapture capture;
#ifdef _WIN32
    if (!capture.open(cameraIndex_, cv::CAP_DSHOW)) {
#else
    if (!capture.open(cameraIndex_)) {
#endif
        error = "failed to open UVC camera index " + std::to_string(cameraIndex_);
        return false;
    }

    cv::Mat frame;
    if (!capture.read(frame) || frame.empty()) {
        error = "camera opened but no frames received at index " + std::to_string(cameraIndex_);
        return false;
    }

    return true;
#else
    error = "OpenCV support not enabled at build time";
    return false;
#endif
}

std::string UvcSource::name() const {
    return "uvc:" + std::to_string(cameraIndex_);
}

} // namespace nanohawk::video
