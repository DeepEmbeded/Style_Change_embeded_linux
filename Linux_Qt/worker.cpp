#include "worker.h"
#include <QDebug>

Worker::Worker(QObject *parent)
    : QObject(parent), running(true) {}

Worker::~Worker() {}

void Worker::stop() {
    running = false;
}

void Worker::process() {
    const std::string dev = "/dev/video11";
    cv::VideoCapture cap(dev, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        qWarning("打开摄像头失败");
        emit finished();
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    running = true;
    cv::Mat frame;

    while (running) {
        if (!cap.read(frame) || frame.empty()) {
            QThread::msleep(10);
            continue;
        }

        // 转 BGR → RGB → QImage
        cv::Mat rgb;
        cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
        QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);

        // 发出原始帧信号（用于推流） — 注意 copy() 确保内存安全
        emit rawFrameReady(qimg.copy());
        qDebug() << "[Worker] 发出原始帧信号";

        // 发给 UI 显示（原样即可）
        emit frameReady(qimg);

        QThread::msleep(33);  // ≈30fps
    }

    cap.release();
    emit finished();
}



