#ifndef YOLOV5_DETECTOR_H
#define YOLOV5_DETECTOR_H

#include <QObject>
#include <QImage>
#include <QString>

// 包含原有的头文件
#include "yolov5.h"
#include "image_utils.h"
#include "image_drawing.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>              // 包含 TrackerCSRT 等
#include <QElapsedTimer>


class YOLOv5Detector : public QObject
{
    Q_OBJECT

public:
    explicit YOLOv5Detector(QObject *parent = nullptr);
    ~YOLOv5Detector();

    // 初始化模型
    Q_INVOKABLE bool initialize(const QString &modelPath, int npu_index);
    void handleTouch(int touchX, int touchY);


public slots:
    // 检测图像
    //void detectImage(const QString &imagePath);
    // 检测QImage
    void detectQImage(const QImage &image);

    // 新增：处理来自摄像头的帧
    void processFrame(const QImage &frame);

signals:
    // 检测完成信号，带结果图片路径
    //void detectionComplete(const QString &resultImagePath);
    // 检测完成信号，带结果QImage
    void detectionComplete(const QImage &resultImage);
    // 错误信号
    void errorOccurred(const QString &errorMessage);
    // 新增：串口信号
    void trackingUpdated(const QRect &rect);  // 通知主窗口坐标更新

    // 新增：FPS信号

    void fpsUpdated(double fps);


private:
    rknn_app_context_t m_rknnAppCtx;
    bool m_initialized;

    //新增：追踪
    cv::Ptr<cv::TrackerCSRT> tracker ;  // ✅ 注意加 legacy 命名空间
    cv::Rect trackedBox;
    bool trackingActive = false;

    // 在 YOLOv5Detector 类中添加一项成员变量（保存当前帧指针）
    image_buffer_t* currentFrame = nullptr;
    extended_result_list detected_results;


    // 初始化后处理
    bool initPostProcess();
    // 释放资源
    void cleanup();

    // 处理检测结果
    void processDetectionResults(image_buffer_t *srcImage, object_detect_result_list *results);
    // 将QImage转换为image_buffer_t
    bool convertQImageToImageBuffer(const QImage &qImage, image_buffer_t &imageBuffer);
    // 释放image_buffer_t资源
    void freeImageBuffer(image_buffer_t &imageBuffer);

    // 新增：触摸检测
    cv::Mat getFrameMatFromBuffer(image_buffer_t* img);

    // 新增：FPS计算
    int m_frameCount = 0;
    QElapsedTimer m_fpsTimer;
    bool m_fpsTimerStarted = false;

};

#endif // YOLOV5_DETECTOR_H
