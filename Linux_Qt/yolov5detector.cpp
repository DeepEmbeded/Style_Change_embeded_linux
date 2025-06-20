#include "yolov5detector.h"
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QPainter>

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

    // 将结果转换回QImage
//    QImage resultImage(srcImage.virt_addr, srcImage.width, srcImage.height,
//                      QImage::Format_RGB888); // 根据实际格式调整
    QImage resultImage(srcImage.virt_addr, srcImage.width, srcImage.height,
                       QImage::Format_BGR888); // ✅ 和你的 QImage 保持一致


    // 复制数据，因为srcImage的内存即将被释放
    QImage resultCopy = resultImage.copy();

    //qDebug() << "[YOLOv5Detector] 推理结果准备完成，发射信号 detectionComplete()";

    emit detectionComplete(resultCopy);

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

        emit fpsUpdated(fps);  // 可用于界面显示

        m_frameCount = 0;
        m_fpsTimer.restart();
    //qDebug() << "[YOLOv5Detector] 释放图像内存完成";
    }
}

void YOLOv5Detector::processDetectionResults(image_buffer_t *srcImage, object_detect_result_list *results)
{
    // 新增：保存当前帧
    currentFrame = srcImage;

    detected_results.count = results->count;

    char text[256];
    for (int i = 0; i < results->count; i++) {
        object_detect_result *detResult = &(results->results[i]);
        extended_detect_result *ext = &(detected_results.results[i]);

        // 拷贝推理信息 + 保留原选中状态（可选：首次设为false）
        ext->id = detResult->cls_id;
        ext->prop = detResult->prop;
        ext->box = detResult->box;
        // 若希望每帧清空选中状态，可写 ext->selected = false;


//        qDebug() << coco_cls_to_name(detResult->cls_id)
//                 << "@ (" << detResult->box.left << detResult->box.top
//                 << detResult->box.right << detResult->box.bottom << ")"
//                 << detResult->prop;

        int x1 = detResult->box.left;
        int y1 = detResult->box.top;
        int x2 = detResult->box.right;
        int y2 = detResult->box.bottom;

        unsigned int boxColor = ext->selected ? COLOR_RED : COLOR_BLUE;


        //draw_rectangle(srcImage, x1, y1, x2 - x1, y2 - y1, COLOR_BLUE, 3);
        draw_rectangle(srcImage, x1, y1, x2 - x1, y2 - y1, boxColor, 3);

        //sprintf(text, "%s %.1f%%", coco_cls_to_name(detResult->cls_id), detResult->prop * 100);
        sprintf(text, "%s %.1f%%", coco_cls_to_name(ext->id), ext->prop * 100);

        draw_text(srcImage, text, x1, y1 - 20, COLOR_RED, 10);
    }

    //新增：如果在追踪，额外花框
    if (trackingActive && tracker) {
            cv::Mat frame = getFrameMatFromBuffer(currentFrame);
            bool ok = tracker->update(frame, trackedBox);
            if (ok) {
                draw_rectangle(srcImage, trackedBox.x, trackedBox.y, trackedBox.width, trackedBox.height, COLOR_RED, 3);
                qDebug() << "Tracking at: x=" << trackedBox.x
                         << " y=" << trackedBox.y
                         << " width=" << trackedBox.width
                         << " height=" << trackedBox.height;
                emit trackingUpdated(QRect(trackedBox.x, trackedBox.y, trackedBox.width, trackedBox.height));


            } else {
                trackingActive = false;
                qDebug() << "Tracking lost";
            }
        }

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

void YOLOv5Detector::handleTouch(int touchX, int touchY) {
    // 取消选中状态
    for (int i = 0; i < detected_results.count; ++i) {
        detected_results.results[i].selected = false;
    }

    // 检查每个检测框是否被点击
    for (int i = 0; i < detected_results.count; ++i) {
        auto& res = detected_results.results[i];
        QRect rect(res.box.left, res.box.top,
                   res.box.right - res.box.left,
                   res.box.bottom - res.box.top);
        if (rect.contains(touchX, touchY)) {
            res.selected = true;
            qDebug() << "Selected box at" << rect;

            // 初始化 Tracker
            tracker = cv::TrackerCSRT::create();  //
            trackedBox = cv::Rect(rect.x(), rect.y(), rect.width(), rect.height());

            if (currentFrame) {
                cv::Mat mat = getFrameMatFromBuffer(currentFrame);
                if (!mat.empty()) {
                    tracker->init(mat, trackedBox);
                    trackingActive = true;
                    qDebug() << "Tracker initialized.";
                } else {
                    trackingActive = false;
                    qDebug() << "Tracker init failed: empty frame.";
                }
            } else {
                qDebug() << "No current frame for tracking.";
            }

            break;
        }
    }
}



cv::Mat YOLOv5Detector::getFrameMatFromBuffer(image_buffer_t* img) {
    if (!img || !img->virt_addr) {
        qDebug() << "image_buffer_t is null";
        return cv::Mat();
    }

    int width = img->width;
    int height = img->height;
    int stride = img->width_stride;
    unsigned char* data = img->virt_addr;

    switch (img->format) {
    case IMAGE_FORMAT_RGB888: {
        cv::Mat rgb(height, width, CV_8UC3, data, stride);
        cv::Mat bgr;
        cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
        return bgr;
    }

    case IMAGE_FORMAT_RGBA8888: {
        cv::Mat rgba(height, width, CV_8UC4, data, stride);
        cv::Mat bgr;
        cv::cvtColor(rgba, bgr, cv::COLOR_RGBA2BGR);
        return bgr;
    }

    case IMAGE_FORMAT_YUV420SP_NV12: {
        cv::Mat yuv(height + height / 2, width, CV_8UC1, data);
        cv::Mat bgr;
        cv::cvtColor(yuv, bgr, cv::COLOR_YUV2BGR_NV12);
        return bgr;
    }

    case IMAGE_FORMAT_YUV420SP_NV21: {
        cv::Mat yuv(height + height / 2, width, CV_8UC1, data);
        cv::Mat bgr;
        cv::cvtColor(yuv, bgr, cv::COLOR_YUV2BGR_NV21);
        return bgr;
    }

    case IMAGE_FORMAT_GRAY8: {
        cv::Mat gray(height, width, CV_8UC1, data, stride);
        cv::Mat bgr;
        cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }

    default:
        qDebug() << "Unsupported image format";
        return cv::Mat();
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
