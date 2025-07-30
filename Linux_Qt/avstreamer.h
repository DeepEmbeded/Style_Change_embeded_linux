#ifndef AVSTREAMER_H
#define AVSTREAMER_H
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <opencv2/opencv.hpp>
#include <QObject>

class AVStreamer : public QObject
{
    Q_OBJECT
public:
    explicit AVStreamer(QObject *parent = nullptr);
    ~AVStreamer();

    bool startStreaming(const std::string& rtmpUrl, int width, int height, int fps = 30);
    void stopStreaming();

    void pushVideoFrame(const cv::Mat& frame);                      // 推送一帧视频
    void pushAudioFrame(const char* data, int length);             // 推送一帧音频（PCM mono）
    bool initPipeline(const std::string& rtmpUrl, int width, int height, int fps);
signals:

public slots:
private:


    GstElement* m_pipeline = nullptr;
    GstElement* m_videoSrc = nullptr;
    GstElement* m_audioSrc = nullptr;

    int m_videoWidth = 1280;
    int m_videoHeight = 720;
    int m_fps = 30;
    guint64 m_videoFrameCount = 0;  // 帧计数器
    guint64 m_audioSampleCount = 0;

    bool m_running = false;
};

#endif // AVSTREAMER_H
