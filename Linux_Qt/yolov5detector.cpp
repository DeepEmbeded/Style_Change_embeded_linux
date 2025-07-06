#include "yolov5detector.h"
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QPainter>
#include "logger.h"

#if defined(RV1106_1103)
#include "dma_alloc.hpp"
#endif

YOLOv5Detector::YOLOv5Detector(QObject *parent)
    : QObject(parent), m_initialized(false)
{
    memset(&m_rknnAppCtx, 0, sizeof(rknn_app_context_t));
}

YOLOv5Detector::~YOLOv5Detector()
{
    cleanup();
}

bool YOLOv5Detector::initialize(const QString &modelPath, int npu_index)
{
    if (m_initialized) {
        cleanup();
    }

    if (!initPostProcess()) {
        emit errorOccurred("Failed to initialize post process");
        return false;
    }

    int ret = init_yolov5_model(modelPath.toStdString().c_str(), &m_rknnAppCtx, npu_index);
    if (ret != 0) {
        emit errorOccurred(QString("init_yolov5_model fail! ret=%1 model_path=%2")
                          .arg(ret).arg(modelPath));
        return false;
    }

    m_initialized = true;
    return true;
}

bool YOLOv5Detector::initPostProcess()
{
    int ret = init_post_process();
    if (ret != 0) {
        emit errorOccurred(QString("init_post_process fail! ret=%1").arg(ret));
        return false;
    }
    return true;
}

void YOLOv5Detector::cleanup()
{
    if (m_initialized) {
        deinit_post_process();

        int ret = release_yolov5_model(&m_rknnAppCtx);
        if (ret != 0) {
            qWarning() << "release_yolov5_model fail! ret=" << ret;
        }

        m_initialized = false;
    }
}


void YOLOv5Detector::detectQImage(const QImage &image)
{
    //qDebug() << "[YOLOv5Detector] detectQImage() 被调用";

    if (!m_initialized) {
        qDebug() << "[YOLOv5Detector] 未初始化！";
        emit errorOccurred("Detector not initialized");
        return;
    }

    image_buffer_t srcImage;
    memset(&srcImage, 0, sizeof(image_buffer_t));

    if (!convertQImageToImageBuffer(image, srcImage)) {
        qDebug() << "[YOLOv5Detector] QImage 转换失败";
        emit errorOccurred("Failed to convert QImage to image buffer");
        return;
    }

    object_detect_result_list odResults;
    int ret = inference_yolov5_model(&m_rknnAppCtx, &srcImage, &odResults);
    if (ret != 0) {
        emit errorOccurred(QString("inference_yolov5_model fail! ret=%1").arg(ret));
        freeImageBuffer(srcImage);
        return;
    }


    processDetectionResults(&srcImage, &odResults);


    QImage bgrImage(srcImage.virt_addr, srcImage.width, srcImage.height,
                    QImage::Format_BGR888);

    // ✅ 转成 RGB 显示用图
    QImage rgbImage = bgrImage.rgbSwapped();  // RGB <=> BGR

    // 发射信号（拷贝安全）
    emit detectionComplete(rgbImage.copy());


    // 释放资源
    freeImageBuffer(srcImage);

    // ===== FPS 统计逻辑开始 =====
    if (!m_fpsTimerStarted) {
        m_fpsTimer.start();
        m_fpsTimerStarted = true;
    }

    m_frameCount++;
    if (m_fpsTimer.elapsed() >= 1000) { // 每秒计算一次
        double fps = m_frameCount * 1000.0 / m_fpsTimer.elapsed();
        qDebug() << "[YOLOv5Detector] 当前FPS:" << fps;
        Logger::instance().appendLine(QString("开始转录，输入帧数: %1").arg(fps));

        emit fpsUpdated(fps);  // 可用于界面显示

        m_frameCount = 0;
        m_fpsTimer.restart();
    qDebug() << "[YOLOv5Detector] 释放图像内存完成";
    }
}





// 处理检测结果，绘制检测框和追踪框
void YOLOv5Detector::processDetectionResults(image_buffer_t *srcImage, object_detect_result_list *results)
{
    QMutexLocker locker(&trackerMutex);

    // 深拷贝图像数据，保证后续使用安全
    if (!srcImage || !srcImage->virt_addr) {
        logToFile("processDetectionResults: srcImage or virt_addr is nullptr");
        currentFrame.release();
        return;
    }

    cv::Mat frameMat = getFrameMatFromBuffer(srcImage);
    if (frameMat.empty()) {
        logToFile("processDetectionResults: getFrameMatFromBuffer returned empty Mat");
        currentFrame.release();
        return;
    }
    currentFrame = frameMat.clone();

    // 拷贝检测结果
    detected_results.count = results->count;
    for (int i = 0; i < results->count; i++) {
        object_detect_result *detResult = &(results->results[i]);
        extended_detect_result *ext = &(detected_results.results[i]);

        ext->id = detResult->cls_id;
        ext->prop = detResult->prop;
        ext->box = detResult->box;
    }

    // 普通绘制逻辑（未激活追踪时）
    if (!trackingActive) {
        for (int i = 0; i < detected_results.count; i++) {
            extended_detect_result *ext = &(detected_results.results[i]);
            int x1 = ext->box.left;
            int y1 = ext->box.top;
            int x2 = ext->box.right;
            int y2 = ext->box.bottom;

            unsigned int boxColor = (i == selectedIndex) ? COLOR_RED : COLOR_BLUE;

            draw_rectangle(srcImage, x1, y1, x2 - x1, y2 - y1, boxColor, 3);

            char text[256];
            sprintf(text, "ID:%d %.1f%%", ext->id, ext->prop * 100);
            draw_text(srcImage, text, x1, y1 - 20, COLOR_RED, 10);
        }
    }

    // ✅ 新的中心点匹配追踪逻辑
    if (trackingActive) {
        int bestIdx = -1;
        float minDist = std::numeric_limits<float>::max();
        QPointF bestCenter;

        for (int i = 0; i < detected_results.count; i++) {
            const auto &box = detected_results.results[i].box;
            QRect rect(box.left, box.top, box.right - box.left, box.bottom - box.top);
            QPointF center = rect.center();
            float dist = QLineF(center, m_targetCenter).length();

            if (dist < minDist) {
                minDist = dist;
                bestIdx = i;
                bestCenter = center;
            }
        }

        const float MAX_TRACK_DIST = 100.0;

        if (bestIdx != -1 && minDist < MAX_TRACK_DIST) {
            const auto &box = detected_results.results[bestIdx].box;
            QRect rect(box.left, box.top, box.right - box.left, box.bottom - box.top);

            // ✅ 对中心点进行指数平滑
            QPointF rawCenter = rect.center();
            static QPointF smoothedCenter = rawCenter;
            const float centerAlpha = 0.6f;
            smoothedCenter = centerAlpha * smoothedCenter + (1.0f - centerAlpha) * rawCenter;

            // ✅ 用平滑中心点生成新的矩形框（保持尺寸不变）
            QSize size = rect.size();
            QPoint topLeft(smoothedCenter.x() - size.width() / 2, smoothedCenter.y() - size.height() / 2);
            QRect smoothedRect(topLeft, size);

            m_targetCenter = smoothedCenter;
            selectedIndex = bestIdx;
            m_lastTrackedRect = smoothedRect;
            m_trackingMissCount = 0;

            draw_rectangle(srcImage, smoothedRect.x(), smoothedRect.y(), smoothedRect.width(), smoothedRect.height(), COLOR_RED, 3);
            emit trackingUpdated(smoothedRect);
        }else {
            // ❌ 本帧未匹配成功
            m_trackingMissCount++;

            if (m_trackingMissCount <= m_trackingMissLimit) {
                // ✅ 使用上一帧追踪框
                draw_rectangle(srcImage, m_lastTrackedRect.x(), m_lastTrackedRect.y(),
                               m_lastTrackedRect.width(), m_lastTrackedRect.height(), COLOR_RED, 3);
                emit trackingUpdated(m_lastTrackedRect);
                logToFile(QString("Tracking fallback to previous rect (miss %1)").arg(m_trackingMissCount));
            } else {
                //  连续丢失太多，终止追踪
                trackingActive = false;
                selectedIndex = -1;
                m_trackingMissCount = 0;
                logToFile("Tracking lost completely, stopping.");
            }
        }
    }

}

//if (bestIdx != -1 && minDist < MAX_TRACK_DIST) {
// ✅ 成功匹配
//const auto &box = detected_results.results[bestIdx].box;
//QRect rect(box.left, box.top, box.right - box.left, box.bottom - box.top);
//m_targetCenter = rect.center();
//selectedIndex = bestIdx;
//m_lastTrackedRect = rect;
//m_trackingMissCount = 0;

//draw_rectangle(srcImage, rect.x(), rect.y(), rect.width(), rect.height(), COLOR_RED, 3);
//emit trackingUpdated(rect);

//}

void YOLOv5Detector::stopTracking()
{
    QMutexLocker locker(&trackerMutex);
    trackingActive = false;
    m_trackingMissCount = 0;
    selectedIndex = -1;
    logToFile("Tracking manually stopped.");
}


bool YOLOv5Detector::convertQImageToImageBuffer(const QImage &qImage, image_buffer_t &imageBuffer)
{
    QImage rgbImage = qImage.convertToFormat(QImage::Format_RGB888);

    // 宽高16字节对齐
    int aligned_width = (rgbImage.width() + 15) & (~15);
    int aligned_height = (rgbImage.height() + 15) & (~15);

    QImage alignedImage(aligned_width, aligned_height, QImage::Format_RGB888);
    alignedImage.fill(Qt::black);

    // 需要QPainter画图，注意加上#include <QPainter>
    QPainter painter(&alignedImage);
    painter.drawImage(0, 0, rgbImage);
    painter.end();

    imageBuffer.width = aligned_width;
    imageBuffer.height = aligned_height;
    imageBuffer.width_stride = alignedImage.bytesPerLine();
    imageBuffer.height_stride = aligned_height;
    imageBuffer.size = alignedImage.sizeInBytes();
    imageBuffer.format = IMAGE_FORMAT_RGB888; // 这里替换为你image_format_t中对应RGB888的枚举值
    imageBuffer.fd = -1;

    imageBuffer.virt_addr = (unsigned char*)malloc(imageBuffer.size);
    if (!imageBuffer.virt_addr) {
        qWarning() << "malloc failed";
        return false;
    }

    memcpy(imageBuffer.virt_addr, alignedImage.bits(), imageBuffer.size);

    return true;
}




void YOLOv5Detector::freeImageBuffer(image_buffer_t &imageBuffer)
{

    if (imageBuffer.virt_addr != NULL) {
        free(imageBuffer.virt_addr);
        imageBuffer.virt_addr = NULL;
    }

}

void YOLOv5Detector::handleTouch(int touchX, int touchY)
{
    logToFile(QString("handleTouch called with: %1 %2").arg(touchX).arg(touchY));

    if (trackingActive) {
        logToFile("Already tracking, ignoring new touch.");
        return;
    }

    logToFile(QString("Clearing selection for %1 boxes").arg(detected_results.count));
    for (int i = 0; i < detected_results.count; ++i) {
        detected_results.results[i].selected = false;
    }

    for (int i = 0; i < detected_results.count; ++i) {
        auto& res = detected_results.results[i];
        QRect rect(res.box.left, res.box.top,
                   res.box.right - res.box.left,
                   res.box.bottom - res.box.top);

        logToFile(QString("Box %1: left=%2, top=%3, right=%4, bottom=%5, QRect=(%6,%7,%8,%9)")
                  .arg(i)
                  .arg(res.box.left)
                  .arg(res.box.top)
                  .arg(res.box.right)
                  .arg(res.box.bottom)
                  .arg(rect.x()).arg(rect.y())
                  .arg(rect.width()).arg(rect.height()));

        if (rect.contains(touchX, touchY)) {
            logToFile(QString("Touch inside box %1").arg(i));

            if (res.selected) {
                logToFile("Box already selected, skipping.");
                return;
            }

            res.selected = true;
            logToFile("Box marked as selected.");

            // ✅ 初始化追踪中心点
            m_targetCenter = rect.center();
            trackingActive = true;
            logToFile(QString("Tracking started at center (%1, %2)")
                      .arg(m_targetCenter.x()).arg(m_targetCenter.y()));

//            emit trackingStarted();  // 可选：通知主界面
            break;
        } else {
            logToFile(QString("Touch not in box %1").arg(i));
        }
    }
}


cv::Mat YOLOv5Detector::getFrameMatFromBuffer(image_buffer_t* img) {
    if (!img) {
        logToFile("getFrameMatFromBuffer: img pointer is nullptr");
        return cv::Mat();
    }
    if (!img->virt_addr) {
        logToFile("getFrameMatFromBuffer: img->virt_addr is nullptr");
        return cv::Mat();
    }
    int width = img->width;
    int height = img->height;
    int stride = img->width_stride;
    unsigned char* data = img->virt_addr;
    logToFile(QString("getFrameMatFromBuffer: width=%1, height=%2, stride=%3, virt_addr=%4, format=%5")
              .arg(img->width)
              .arg(img->height)
              .arg(img->width_stride)
              .arg((quintptr)img->virt_addr, 0, 16)
              .arg(img->format));

    switch (img->format) {
    case IMAGE_FORMAT_RGB888: {
        cv::Mat rgb(height, width, CV_8UC3, data, stride);
        cv::Mat bgr;
        cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
        return bgr.clone();  // ✅ clone 关键
    }

    case IMAGE_FORMAT_RGBA8888: {
        cv::Mat rgba(height, width, CV_8UC4, data, stride);
        cv::Mat bgr;
        cv::cvtColor(rgba, bgr, cv::COLOR_RGBA2BGR);
        return bgr.clone();
    }

    case IMAGE_FORMAT_YUV420SP_NV12: {
        cv::Mat yuv(height + height / 2, width, CV_8UC1, data);
        cv::Mat bgr;
        cv::cvtColor(yuv, bgr, cv::COLOR_YUV2BGR_NV12);
        return bgr.clone();
    }

    case IMAGE_FORMAT_YUV420SP_NV21: {
        cv::Mat yuv(height + height / 2, width, CV_8UC1, data);
        cv::Mat bgr;
        cv::cvtColor(yuv, bgr, cv::COLOR_YUV2BGR_NV21);
        return bgr.clone();
    }

    case IMAGE_FORMAT_GRAY8: {
        cv::Mat gray(height, width, CV_8UC1, data, stride);
        cv::Mat bgr;
        cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);
        return bgr.clone();
    }

    default:
        qDebug() << "Unsupported image format";
        return cv::Mat();
    }
}


void YOLOv5Detector::logToFile(const QString &message)
{
    QFile file("touch_debug.log");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
            << " - " << message << "\n";
    }
}

// yolov5_detector.cpp
void YOLOv5Detector::processFrame(const QImage &frame)
{
    if (!m_initialized) {
        qWarning() << "Detector not initialized, cannot process frame";
        return;
    }

    // 直接调用已有的detectQImage方法
    detectQImage(frame);

    // 注意: detectionComplete信号会被触发，由外部连接处理
}
