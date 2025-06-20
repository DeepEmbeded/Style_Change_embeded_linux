#include "worker.h"

Worker::Worker(QObject *parent)
    : QObject(parent), running(true) {}

Worker::~Worker() {}

void Worker::stop() {
    running = false;
}

void Worker::process() {
    // 摄像头设备
    const std::string dev = "/dev/video11";

    // GStreamer推流管道，使用mpph264enc，注意尺寸和帧率根据实际调整
    std::string gst_pipeline =
        "appsrc ! "
        "videoconvert ! "
        "video/x-raw,format=I420 ! "    // mpph264enc 要求格式，opencv默认NV12，转I420格式
        "mpph264enc ! "
        "h264parse ! "
        "flvmux ! "
        "rtmpsink location=rtmp://192.168.10.50/live/stream";

//    gst-launch-1.0 \
//      appsrc ! videoconvert ! x264enc tune=zerolatency ! rtph264pay ! \
//      whipclientsink url=https://your-webrtc-server/whip/endpoint


    cv::VideoCapture cap(dev, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        qWarning("打开摄像头失败");
        emit finished();
        return;
    }

    // 设置摄像头分辨率（根据实际摄像头调整）
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('N','V','1','2')); // NV12

    // 打开VideoWriter推流
    cv::VideoWriter writer;
    bool opened = writer.open(gst_pipeline, 0, 30.0, cv::Size(1280, 720), true);
    if (!opened) {
        qWarning("无法打开GStreamer推流管道");
        cap.release();
        emit finished();
        return;
    }

    cv::Mat frame;
    QImage qimg;

    running = true;

    while (running) {
        if (!cap.read(frame) || frame.empty()) {
            QThread::msleep(10);

            continue;
        }

        if (frame.channels() != 3) {
            cv::cvtColor(frame, frame, cv::COLOR_YUV2BGR_NV12);
        }
        // 传给QImage
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);
        emit frameReady(qimg);

        // 写入推流管道，推流管线中加 videoconvert 转换格式
        writer.write(frame);

        // 控制帧率
        QThread::msleep(10);
        // 30ms约33fps
    }

    writer.release();
    cap.release();

    emit finished();
}

