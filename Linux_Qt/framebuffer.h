#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <QMutex>
#include <opencv2/opencv.hpp>

class FrameBuffer {
public:
    void updateFrame(const cv::Mat &frame) {
        QMutexLocker locker(&m_mutex);
        m_frame = frame.clone();  // 拷贝一份安全存储
    }

    cv::Mat getFrame() {
        QMutexLocker locker(&m_mutex);
        return m_frame.clone();   // 返回拷贝，避免引用同一块内存
    }

private:
    cv::Mat m_frame;
    QMutex m_mutex;
};

#endif // FRAMEBUFFER_H
