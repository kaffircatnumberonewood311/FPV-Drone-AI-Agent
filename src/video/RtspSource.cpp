#include "video/RtspSource.hpp"

#ifdef NANOHAWK_WITH_OPENCV
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#endif

namespace nanohawk::video {

RtspSource::RtspSource(std::string uri) : uri_(std::move(uri)) {}

bool RtspSource::open(std::string& error) {
    if (uri_.empty()) {
        error = "RTSP URI is empty";
        return false;
    }

#ifdef NANOHAWK_WITH_OPENCV
    cv::VideoCapture capture;
    if (!capture.open(uri_)) {
        error = "failed to open RTSP/video source: " + uri_;
        return false;
    }

    cv::Mat frame;
    if (!capture.read(frame) || frame.empty()) {
        error = "source opened but no frames received from: " + uri_;
        return false;
    }

    return true;
#else
    error = "OpenCV support not enabled at build time";
    return false;
#endif
}

std::string RtspSource::name() const {
    return "rtsp:" + uri_;
}

} // namespace nanohawk::video

