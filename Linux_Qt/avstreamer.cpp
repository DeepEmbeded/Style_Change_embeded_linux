#include "avstreamer.h"
#include <gst/video/video.h>
#include <QDebug>

AVStreamer::AVStreamer(QObject* parent) : QObject(parent)
{
    gst_init(nullptr, nullptr);
}

AVStreamer::~AVStreamer()
{
    stopStreaming();
}
//    std::string pipelineStr =
//        "appsrc name=videosrc is-live=true do-timestamp=true format=3 ! "
//        "video/x-raw,format=BGR,width=" + std::to_string(width) + ",height=" + std::to_string(height) +
//        ",framerate=" + std::to_string(fps) + "/1 ! "
//        "videoconvert ! video/x-raw,format=I420 ! mpph264enc ! h264parse ! "
//        "flvmux streamable=true ! "
//        "rtmpsink location=" + rtmpUrl;
bool AVStreamer::startStreaming(const std::string& rtmpUrl, int width, int height, int fps)
{
    m_videoWidth = width;
    m_videoHeight = height;
    m_fps = fps;

    return initPipeline(rtmpUrl, width, height, fps);
}

void AVStreamer::stopStreaming()
{
    if (m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
        m_videoSrc = nullptr;
        m_audioSrc = nullptr;
        m_running = false;
    }
}

bool AVStreamer::initPipeline(const std::string& rtmpUrl, int width, int height, int fps)
{
    m_fps = fps;  // 保存帧率用于后续 PTS 计算

    std::string pipelineStr =
        "appsrc name=videosrc is-live=true format=3 ! "
        "video/x-raw,format=BGR,width=" + std::to_string(width) +
        ",height=" + std::to_string(height) +
        ",framerate=" + std::to_string(fps) + "/1 ! "
        "videoconvert ! video/x-raw,format=I420 ! "
        "mpph264enc ! h264parse ! queue ! mux. "

        "appsrc name=audiosrc is-live=true format=3 ! "
        "audio/x-raw,format=S16LE,channels=1,rate=16000 ! "
        "voaacenc ! queue ! mux. "

        "flvmux name=mux streamable=true ! "
        "rtmpsink location=" + rtmpUrl;

//    std::string pipelineStr =
//        "appsrc name=videosrc is-live=true format=3 ! "
//        "video/x-raw,format=BGR,width=" + std::to_string(width) +
//        ",height=" + std::to_string(height) +
//        ",framerate=" + std::to_string(fps) + "/1 ! "
//        "videoconvert ! video/x-raw,format=I420 ! "
//        "mpph264enc ! h264parse ! queue ! mux. "

//        "appsrc name=audiosrc is-live=true format=3 ! "
//        "audio/x-raw,format=S16LE,channels=1,rate=16000 ! "
//        "audioconvert ! audioresample ! audio/x-raw,format=S16LE,rate=48000,channels=2 !"
//        "fdkaac ! queue ! mux. "

//        "flvmux name=mux streamable=true ! "
//        "rtmpsink location=" + rtmpUrl;


    GError* error = nullptr;
    m_pipeline = gst_parse_launch(pipelineStr.c_str(), &error);
    if (!m_pipeline || error) {
        qWarning() << "[AVStreamer] GStreamer pipeline 构建失败:" << (error ? error->message : "未知错误");
        if (error) g_error_free(error);
        return false;
    }

    // 获取 appsrc 元素
    m_videoSrc = gst_bin_get_by_name(GST_BIN(m_pipeline), "videosrc");
    m_audioSrc = gst_bin_get_by_name(GST_BIN(m_pipeline), "audiosrc");
    if (!m_videoSrc || !m_audioSrc) {
        qWarning() << "[AVStreamer] 无法获取 videosrc 或 audiosrc";
        return false;
    }

    // 设置 pipeline 状态为 PLAYING
    GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "[AVStreamer] 设置 pipeline 状态为 PLAYING 失败";
        return false;
    }else {
        qDebug() << "[AVStreamer] 设置 pipeline 为 PLAYING 成功：" << ret;
    }
//    GstBus* bus = gst_element_get_bus(m_pipeline);
//    gst_bus_add_signal_watch(bus);
//    g_signal_connect(bus, "message", G_CALLBACK(+[](GstBus* bus, GstMessage* message, gpointer user_data) {
//        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
//            GError* err;
//            gchar* debug;
//            gst_message_parse_error(message, &err, &debug);
//            qWarning() << "[GStreamer ERROR]" << err->message;
//            g_error_free(err);
//            g_free(debug);
//        }
//    }), nullptr);

    // 初始化计数器
    m_videoFrameCount = 0;
    m_audioSampleCount = 0;
    m_running = true;

    qDebug() << "[AVStreamer] 推流管道已启动";

    // 推送一帧黑图
    {
        cv::Mat black(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
        pushVideoFrame(black);
    }

    // 推送10ms静音（16bit单声道16000Hz => 320字节）
    {
        std::vector<char> silence(320, 0);
        pushAudioFrame(silence.data(), silence.size());
    }

    return true;
}


void AVStreamer::pushVideoFrame(const cv::Mat& bgrFrame)
{
    if (!m_running || !m_videoSrc || bgrFrame.empty()) {
            qWarning() << "[AVStreamer] 视频推送条件不满足";
            return;
        }
//    qDebug() << "[AVStreamer] 正在推送视频帧 PTS:" << m_videoFrameCount;

    const int dataSize = bgrFrame.total() * bgrFrame.elemSize();

    GstBuffer* buffer = gst_buffer_new_allocate(nullptr, dataSize, nullptr);
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
        memcpy(map.data, bgrFrame.data, dataSize);
        gst_buffer_unmap(buffer, &map);
    } else {
        gst_buffer_unref(buffer);
        return;
    }

    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(m_videoFrameCount, GST_SECOND, m_fps);
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1, GST_SECOND, m_fps);
    m_videoFrameCount++;

    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(m_videoSrc), buffer);
    if (ret != GST_FLOW_OK) {
        qWarning() << "[AVStreamer] 视频推送失败:" << ret;
    }
}



void AVStreamer::pushAudioFrame(const char* data, int length)
{
//    qDebug() << "[AVStreamer] 收到音频数据，长度：" << length;

    if (!m_running || !m_audioSrc || !data || length <= 0) {
        qWarning() << "[AVStreamer] 音频数据无效: m_running=" << m_running
                       << ", m_audioSrc=" << (m_audioSrc ? "OK" : "nullptr")
                       << ", length=" << length;
        return;
    }

    GstBuffer* buffer = gst_buffer_new_allocate(nullptr, length, nullptr);
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
        memcpy(map.data, data, length);
        gst_buffer_unmap(buffer, &map);
    } else {
        qWarning() << "[AVStreamer] 音频buffer映射失败";
        gst_buffer_unref(buffer);
        return;
    }

    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(m_audioSampleCount, GST_SECOND, 16000);
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(length / 2, GST_SECOND, 16000);
    m_audioSampleCount += length / 2;

    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(m_audioSrc), buffer);
    if (ret != GST_FLOW_OK) {
        qWarning() << "[AVStreamer] 音频推送失败: " << ret;
    } else {
//        qDebug() << "[AVStreamer] 成功推送音频帧，PTS:" << m_audioSampleCount;
    }
}




