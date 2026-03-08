#pragma once

#ifdef NANOHAWK_WITH_GUI
#include <QWidget>
#include <memory>

class QLabel;
class QTimer;

namespace cv {
class VideoCapture;
}

namespace nanohawk::gui {

class VideoPane final : public QWidget {
public:
    VideoPane();
    ~VideoPane() override;

private:
    int loadCameraIndex() const;
    void updateFrame();

    QLabel* statusLabel_{nullptr};
    QLabel* frameLabel_{nullptr};
    QTimer* frameTimer_{nullptr};
    std::unique_ptr<cv::VideoCapture> capture_;
};

} // namespace nanohawk::gui
#endif

