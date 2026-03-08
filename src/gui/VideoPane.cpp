#include "gui/VideoPane.hpp"

#ifdef NANOHAWK_WITH_GUI
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QRegularExpression>
#include <QTimer>
#include <QVBoxLayout>
#include <QStringList>

#ifdef NANOHAWK_WITH_OPENCV
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#endif

namespace nanohawk::gui {

VideoPane::VideoPane() {
    auto* layout = new QVBoxLayout(this);

    statusLabel_ = new QLabel(this);
    frameLabel_ = new QLabel(this);
    frameLabel_->setMinimumSize(640, 360);
    frameLabel_->setAlignment(Qt::AlignCenter);

    layout->addWidget(statusLabel_);
    layout->addWidget(frameLabel_, 1);

#ifdef NANOHAWK_WITH_OPENCV
    const int cameraIndex = loadCameraIndex();

    capture_ = std::make_unique<cv::VideoCapture>();
#ifdef _WIN32
    const bool opened = capture_->open(cameraIndex, cv::CAP_DSHOW);
#else
    const bool opened = capture_->open(cameraIndex);
#endif

    if (!opened) {
        statusLabel_->setText(QString("Camera open failed (uvc_index=%1)").arg(cameraIndex));
        frameLabel_->setText("No camera feed");
        return;
    }

    statusLabel_->setText(QString("Live camera feed (uvc_index=%1)").arg(cameraIndex));

    frameTimer_ = new QTimer(this);
    connect(frameTimer_, &QTimer::timeout, this, [this]() { updateFrame(); });
    frameTimer_->start(33);
#else
    statusLabel_->setText("OpenCV support is disabled in this build");
    frameLabel_->setText("No camera feed");
#endif
}

VideoPane::~VideoPane() {
#ifdef NANOHAWK_WITH_OPENCV
    if (capture_ && capture_->isOpened()) {
        capture_->release();
    }
#endif
}

int VideoPane::loadCameraIndex() const {
    const QStringList candidates = {
        "config/endpoints.yaml",
        "../config/endpoints.yaml",
        "../../config/endpoints.yaml"
    };

    const QRegularExpression expr("^\\s*uvc_index\\s*:\\s*(\\d+)\\s*$");

    for (const QString& path : candidates) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        while (!file.atEnd()) {
            const QString line = QString::fromUtf8(file.readLine());
            const QRegularExpressionMatch match = expr.match(line);
            if (match.hasMatch()) {
                bool ok = false;
                const int value = match.captured(1).toInt(&ok);
                if (ok && value >= 0) {
                    return value;
                }
            }
        }
    }

    return 0;
}

void VideoPane::updateFrame() {
#ifdef NANOHAWK_WITH_OPENCV
    if (!capture_ || !capture_->isOpened()) {
        return;
    }

    cv::Mat frame;
    if (!capture_->read(frame) || frame.empty()) {
        return;
    }

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);

    QImage image(rgb.data,
                 rgb.cols,
                 rgb.rows,
                 static_cast<int>(rgb.step),
                 QImage::Format_RGB888);

    const QPixmap pixmap = QPixmap::fromImage(image);
    frameLabel_->setPixmap(pixmap.scaled(frameLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
#endif
}

} // namespace nanohawk::gui
#endif

