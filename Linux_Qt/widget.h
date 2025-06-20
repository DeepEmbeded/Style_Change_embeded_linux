#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include <QImage>
#include "worker.h"
#include "yolov5detector.h"
#include "serialportmanager.h"  // 添加串口头文件
#include "mqttreceiver.h"
#include <QMutex>
#include "micrecorder.h"
#include "whisperworker.h"
#include <QtMqtt/QMqttClient>



class Worker;
class YOLOv5Detector;
class MicRecorder;
class WhisperWorker;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
private slots:
    void updateFrame(const QImage &frame);
    void updateInferResult(const QImage &result);
    void handleError(const QString &error);
    void onStartClicked();
    void onStopClicked();
    void onWorkerFinished();

    //新增：串口
    void onSerialError(const QString &error);
    void onPortOpened();

    void onWhisperResultReady(const QString &text);

    //新增：推推理流
    void initInferStreamer();
private:
    Ui::Widget *ui;
    QThread *m_workerThread;
    QThread *m_detectorThread;
    Worker *m_worker;
    YOLOv5Detector *m_detector;

    //新增
    QThread *micThread;

    MicRecorder *mic;

    // ✅ Whisper 推理线程和工作对象
    QThread *whisperThread;              // 外部线程
    WhisperWorker *whisperworker;        // 推理工作对象


    QMutex m_frameMutex;
    QImage m_lastFrame;

    // 初始化线程和连接
    void initThreads();
    // 清理资源
    void cleanup();
    void startDetectionPipeline();

    //新增：启用触摸
    bool m_touchEnabled = true; // 是否启用触摸
    bool eventFilter(QObject *watched, QEvent *event) override;

    //新增：串口通信
private:
    SerialPortManager *m_serialManager;
    // 新增： mqtt
    MqttReceiver *mqttReceiver;
    QMqttClient *m_mqttClient;

    //新增：双推
    cv::VideoWriter m_inferWriter;
    bool m_inferWriterOpened = false;

};

#endif // WIDGET_H
