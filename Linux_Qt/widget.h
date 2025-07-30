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
#include "llmworker.h"
#include <QQueue>
#include "avstreamer.h"
extern "C" {
    #include <gst/gst.h>
    #include <gst/app/gstappsrc.h>
}



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
//    void onStopClicked();
    void onWorkerFinished();

    //新增：串口
    void onSerialError(const QString &error);
    void onPortOpened();

    void onWhisperResultReady(const QString &text);

    //新增：推推理流
    void initInferStreamer();

    void onCancelTrackingClicked();


    void on_btnllm_pressed();

    void on_btnllm_released();
    void onLlmResultReady(const QString &text);
    void on_exitbt_clicked();
    void onMqttMessageReceived(const QMqttMessage &message);
    void onSummarizeButtonClicked();
//    void onPcmFrameReady(const char* data, int length);
//    void onRawVideoFrameReady(const cv::Mat& frame);
private slots:
    void onRawVideoFrameReady(const QImage& image);

private:
    Ui::Widget *ui;
    // 线程指针及对象成员
    QThread* m_workerThread = nullptr;
    Worker* m_worker = nullptr;

    QThread* m_detectorThread = nullptr;
    YOLOv5Detector* m_detector = nullptr;

    QThread* micThread = nullptr;
    MicRecorder* mic = nullptr;

    QThread* whisperThread = nullptr;
    WhisperWorker* whisperworker = nullptr;


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

private:
    SerialPortManager* m_serialManager = nullptr;
    MqttReceiver* mqttReceiver = nullptr;
    QMqttClient* m_mqttClient = nullptr;
    AVStreamer* streamer = nullptr;

    //新增：双推
    cv::VideoWriter m_inferWriter;
    bool m_inferWriterOpened = false;
    void initUI();
    void initSerialPort();
    void initButtons();
    void initMQTT();
    void initStreamer();

    void initVideoCaptureThread();
    void initDetectorThread();
    void initMicThread();
    void initWhisperThread();
    void initConnections();
    void initDetectorModel();
    void initllm();

    QPoint mapMqttCoordToImage(int x, int y, QSize labelSize, QSize imageSize);
    void logToFile(const QString &message);

    //新增大模型
    QThread *m_llmThread;
    LLMWorker *m_llmWorker;
//    QString m_lastChineseText;
//    QQueue<QString> m_chineseTextQueue;
    QString m_lastWhisperText;

//    QString m_llmPartialResult;
    bool m_llmActive = false;
    QStringList m_whisperTextBuffer;  // 长期缓存识别内容
signals:
    void sendTextToLLM(const QString &text);


};

#endif // WIDGET_H
