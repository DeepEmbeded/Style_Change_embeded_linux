#include "widget.h"
#include "ui_widget.h"
#include "worker.h"
#include "yolov5detector.h"
#include <QDebug>
#include <QTimer>
#include <QTouchEvent>
#include <QJsonObject>
#include <QJsonDocument>
#include "threadutils.h"


const float HFOV = 65.0f; // 水平视场角（度）
const float VFOV = 60.0f; // 垂直视场角（度）
template<typename T>
static inline T clamp(T val, T minVal, T maxVal) {
    return std::min(std::max(val, minVal), maxVal);
}
// Widget.cpp

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->showFullScreen();

    initUI();
    initSerialPort();
    initButtons();
    initMQTT();
    initThreads();
}

void Widget::initUI()
{
    ui->inferLabel->setAttribute(Qt::WA_AcceptTouchEvents);
    ui->inferLabel->installEventFilter(this);
    ui->inferLabel->setAlignment(Qt::AlignCenter);
    ui->inferLabel->setText("等待启动...");
}

void Widget::initSerialPort()
{
    m_serialManager = new SerialPortManager(this);
    connect(m_serialManager, &SerialPortManager::errorOccurred, this, &Widget::onSerialError);
    connect(m_serialManager, &SerialPortManager::portOpened, this, &Widget::onPortOpened);
    m_serialManager->openPort("/dev/ttyS9", QSerialPort::Baud115200);
}

void Widget::initButtons()
{
    connect(ui->btnStart, &QPushButton::clicked, this, &Widget::onStartClicked);
//    connect(ui->btnStop, &QPushButton::clicked, this, &Widget::onStopClicked);
    connect(ui->cancelTrackingButton, &QPushButton::clicked, this, &Widget::onCancelTrackingClicked);
    connect(ui->exitbt, &QPushButton::clicked, this, &Widget::on_exitbt_clicked);
    connect(ui->SummarizeButton, &QPushButton::clicked, this, &Widget::onSummarizeButtonClicked);
}

void Widget::initMQTT()
{
    // 你已有的mqttReceiver，保持不动（如果没用到可删）
    mqttReceiver = new MqttReceiver(this);
//    mqttReceiver->connectToBroker("192.168.10.200", 1883);
    mqttReceiver->connectToBroker("192.168.184.52", 1883);
    qDebug() << "[MQTT] 正在连接至 Broker: 192.168.10.200:1883";

    // QMqttClient初始化
    m_mqttClient = new QMqttClient(this);
//    m_mqttClient->setHostname("192.168.10.200");
    m_mqttClient->setHostname("192.168.184.52");
    m_mqttClient->setPort(1883);
    m_mqttClient->connectToHost();

    connect(m_mqttClient, &QMqttClient::connected, this, [=]() {
        qDebug() << "[MQTT] 已连接到 Broker";

        // 连接成功后订阅取消跟踪主题
        const QString topic = "button/cancel";
        QMqttSubscription *subscription = m_mqttClient->subscribe(topic);
        if (!subscription) {
            qWarning() << "[MQTT] 订阅主题失败:" << topic;
            return;
        }
        qDebug() << "[MQTT] 已订阅主题:" << topic;

        // 连接订阅消息槽
        connect(subscription, &QMqttSubscription::messageReceived,
                this, &Widget::onMqttMessageReceived);
    });

    connect(m_mqttClient, &QMqttClient::disconnected, this, [] {
        qDebug() << "[MQTT] 与 Broker 断开连接";
    });

    // 连接 fps 更新槽，确保 m_detector 已初始化
    if (m_detector) {
        connect(m_detector, &YOLOv5Detector::fpsUpdated, this, [=](double fps) {
            ui->inferLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 2));
        });
    }
}
void Widget::onMqttMessageReceived(const QMqttMessage &message)
{
    QString payload = QString::fromUtf8(message.payload()).trimmed();
    qDebug() << "[MQTT] 收到主题:" << message.topic() << "消息:" << payload;

    if (payload.compare("cancel", Qt::CaseInsensitive) == 0) {
        if (m_detector) {
            m_detector->stopTracking();
            qDebug() << "[MQTT] 收到停止跟踪指令，已停止追踪";
        }
    }
}

Widget::~Widget()
{
    cleanup();
    delete ui;
}

void Widget::updateFrame(const QImage &frame)
{
    // 初始化检测器模型
    //qDebug() << "收到新视频帧，尺寸:" << frame.size();
    // 保存最新帧
    {
        QMutexLocker locker(&m_frameMutex);
        m_lastFrame = frame.copy();
    }

    // 显示原始帧
//    QPixmap pixmap = QPixmap::fromImage(frame)
//                    .scaled(ui->videoLabel->size(), Qt::KeepAspectRatio);
//    ui->videoLabel->setPixmap(pixmap);

    // 传递给检测器
    if (m_detector) {
        //qDebug() << "传递帧给检测器";

        QMetaObject::invokeMethod(m_detector, "processFrame",
                                Qt::QueuedConnection,
                                Q_ARG(QImage, frame));
    }
}

void Widget::updateInferResult(const QImage &result)
{
    // 1. 显示到 inferLabel 上
    QPixmap pixmap = QPixmap::fromImage(result)
                     .scaled(ui->inferLabel->size(), Qt::KeepAspectRatio);
    ui->inferLabel->setPixmap(pixmap);  // ✅ 重要！缺这行不会显示

    // 2. 可选：保存实际显示尺寸（如果需要用于坐标转换）
    QSize m_displaySize = ui->inferLabel->size();
    int display_width = pixmap.width();
    int display_height = pixmap.height();
    // qDebug() << "显示分辨率：" << display_width << "x" << display_height;

    // 3. 转为 BGR 图像推理帧推流
    QImage img = result.copy().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(img.height(), img.width(), CV_8UC3,
                const_cast<uchar*>(img.bits()), img.bytesPerLine());
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);

    if (m_inferWriterOpened) {
        m_inferWriter.write(bgr);
    }

    // qDebug() << "推理帧尺寸：" << img.width() << "x" << img.height();
}

void Widget::handleError(const QString &error)
{
    // 初始化检测器模型
    qDebug() << "发生错误：" << error;
    ui->inferLabel->setText("错误: " + error);
}

void Widget::onStartClicked()
{
    // 初始化检测器模型
    //qDebug() << "[MainWindow] 初始化检测器模型...";
    startDetectionPipeline();
}

//void Widget::onStopClicked()
//{
//    qDebug() << "[Widget] Stop clicked, stopping workers and threads...";

//    // 1. 告诉工作对象停止（如果有stop方法，且线程对象不为空）
//    if (m_worker) {
//        QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);
//    }
//    if (whisperworker) {
//        QMetaObject::invokeMethod(whisperworker, "stop", Qt::QueuedConnection);
//    }
//    if (m_llmWorker) {
//        QMetaObject::invokeMethod(m_llmWorker, "stop", Qt::QueuedConnection);
//    }
//    if (mic) {
//        QMetaObject::invokeMethod(mic, "stop", Qt::QueuedConnection);
//    }


//    // 2. 等待线程退出并清理资源
//    cleanup();
//}

void Widget::onWorkerFinished()
{
    // 初始化检测器模型
    //qDebug() << "[MainWindow] 初始化检测器模型...";
}



void Widget::initThreads()
{
    initVideoCaptureThread();
    initDetectorThread();
    initMicThread();
    initWhisperThread();
    initConnections();
    initInferStreamer();
    initDetectorModel();
    initllm();
    m_workerThread->start(QThread::HighPriority);
    m_detectorThread->start(QThread::HighPriority);
    micThread->start(QThread::HighPriority);
    whisperThread->start(QThread::HighPriority);
    m_llmThread->start();
}

void Widget::initVideoCaptureThread()
{
    m_workerThread = new QThread(this);
    m_worker = new Worker();
    m_worker->moveToThread(m_workerThread);
}

void Widget::initDetectorThread()
{
    m_detectorThread = new QThread(this);
    m_detector = new YOLOv5Detector();
    m_detector->moveToThread(m_detectorThread);
}

void Widget::initMicThread()
{
    micThread = new QThread(this);
    mic = new MicRecorder();
    mic->moveToThread(micThread);
    connect(micThread, &QThread::finished, mic, &QObject::deleteLater);
    connect(micThread, &QThread::started, mic, &MicRecorder::start);
}

void Widget::initWhisperThread()
{
    whisperThread = new QThread(this);
    whisperworker = new WhisperWorker();
    whisperworker->moveToThread(whisperThread);

    connect(whisperThread, &QThread::started, whisperworker, &WhisperWorker::processLoop);
    connect(whisperThread, &QThread::finished, whisperworker, &QObject::deleteLater);
}

void Widget::initConnections()
{
    // mic线程
    connect(micThread, &QThread::started, [=]() {
        bindThreadToCore(micThread, 4);
    });
    // whisper线程
    connect(whisperThread, &QThread::started, [=]() {
        bindThreadToCore(whisperThread, 5);
    });
    //视频采集
    connect(m_workerThread, &QThread::started, [=]() {
        bindThreadToCore(m_workerThread, 6);
    });
    //推理
    connect(m_detectorThread, &QThread::started, [=]() {
        bindThreadToCore(m_detectorThread, 7);
    });

    connect(m_worker, &Worker::frameReady, this, &Widget::updateFrame);
    connect(m_worker, &Worker::finished, this, &Widget::onWorkerFinished);

    connect(m_detector, &YOLOv5Detector::detectionComplete, this, &Widget::updateInferResult);
    connect(m_detector, &YOLOv5Detector::errorOccurred, this, &Widget::handleError);

    connect(mic, &MicRecorder::audioSegmentReady,
                      whisperworker, &WhisperWorker::pushAudioData,
                      Qt::QueuedConnection);
//    qDebug() << "connect audioSegmentReady to pushAudioData result:" << ok;

    connect(whisperworker, &WhisperWorker::resultReady,
            this, &Widget::onWhisperResultReady);
    connect(whisperworker, &WhisperWorker::resultReady, this, [](const QString &res){
        qDebug() << "[WhisperWorker] 识别结果:" << res;
    });

    connect(mqttReceiver, &MqttReceiver::coordinateReceived,
            m_detector, &YOLOv5Detector::handleTouch, Qt::QueuedConnection);


    // 添加静态变量：帧间滤波状态
    static float last_angle_x = 0.0f;
    static float last_angle_y = 0.0f;

    // 放在类成员里或函数外部（保持历史滤波状态）

    connect(m_detector, &YOLOv5Detector::trackingUpdated, this, [=](const QRect &rect) mutable {
        constexpr int DISP_WIDTH = 1280;
        constexpr int DISP_HEIGHT = 720;

        float cx = rect.x() + rect.width() / 2.0f;
        float cy = rect.y() + rect.height() / 2.0f;
        float dx = cx - DISP_WIDTH / 2.0f;
        float dy = cy - DISP_HEIGHT / 2.0f;

        float ndx = dx / (DISP_WIDTH / 2.0f);
        float ndy = dy / (DISP_HEIGHT / 2.0f);

        float angle_x = -ndx * HFOV;
        float angle_y =  ndy * VFOV;

        // 死区
        const float DEAD_ZONE_X = 2.0f;
        const float DEAD_ZONE_Y = 4.0f;
        if (std::abs(angle_x) < DEAD_ZONE_X) angle_x = 0;
        if (std::abs(angle_y) < DEAD_ZONE_Y) angle_y = 0;

        // 滤波
        static float last_angle_x = 0.0f;
        static float last_angle_y = 0.0f;
        const float alpha = 0.6f;
        angle_x = alpha * last_angle_x + (1 - alpha) * angle_x;
        angle_y = alpha * last_angle_y + (1 - alpha) * angle_y;
        last_angle_x = angle_x;
        last_angle_y = angle_y;

        // 限幅保护
        angle_x = clamp(angle_x, -80.0f, 80.0f);
        angle_y = clamp(angle_y, -35.0f, 35.0f);

        QString data = QString("AX:%1 AY:%2\n")  // ✅ AX ← angle_y，AY ← angle_x
                           .arg(QString::number(angle_y, 'f', 2))  // pitch -> AX
                           .arg(QString::number(angle_x, 'f', 2)); // yaw   -> AY
        m_serialManager->sendData(data.toUtf8());

    });



}

void Widget::initDetectorModel()
{
    QMetaObject::invokeMethod(m_detector, "initialize", Qt::QueuedConnection,
                              Q_ARG(QString, "/home/elf/model/RK3588/yolov5s-640-640.rknn"),
                              Q_ARG(int, 0));
}

void Widget::initllm()
{
    m_llmThread = new QThread(this);
    m_llmWorker = new LLMWorker();
    m_llmWorker->moveToThread(m_llmThread);

    m_llmWorker->setModelPath("/home/elf/model/DeepSeek-R1-Distill-Qwen-1.5B_W8A8_RK3588.rkllm");
    m_llmWorker->setTokenParams(128, 512);

    connect(m_llmThread, &QThread::started, m_llmWorker, &LLMWorker::init);

    connect(m_llmWorker, &LLMWorker::initialized, this, [=](bool success){
        if (success) {
            qDebug() << "[LLM] 初始化完成";
        } else {
            qDebug() << "[LLM] 初始化失败";
        }
    });

    connect(m_llmWorker, &LLMWorker::llmResultReady, this, &Widget::onLlmResultReady);
    connect(this, &Widget::sendTextToLLM,
            m_llmWorker, &LLMWorker::runPrompt);

}


QPoint Widget::mapMqttCoordToImage(int x, int y, QSize labelSize, QSize imageSize)
{
    float scaleX = (float)imageSize.width() / labelSize.width();
    float scaleY = (float)imageSize.height() / labelSize.height();
    return QPoint(x * scaleX, y * scaleY);
}



void Widget::cleanup()
{
    qDebug() << "清理资源";

    // 1. WhisperWorker
    if (whisperworker) {
        whisperworker->deleteLater();  // ✅ 用 deleteLater 而不是 delete
        whisperworker = nullptr;
    }

    // 2. Audio micThread
    if (micThread) {
        micThread->quit();
        micThread->wait();
        micThread->deleteLater();  // ✅ 推荐使用 deleteLater
        micThread = nullptr;
    }

    // 3. Detector Thread
    if (m_detectorThread) {
        m_detectorThread->quit();
        m_detectorThread->wait();
        m_detectorThread->deleteLater();  // ✅ 更安全
        m_detectorThread = nullptr;
    }

    // 4. Worker Thread
    if (m_worker) {
        m_worker->deleteLater();   // ✅ 先释放 QObject
        m_worker = nullptr;
    }
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        m_workerThread->deleteLater();  // ✅ 更安全
        m_workerThread = nullptr;
    }

    // 5. LLM Thread
    if (m_llmWorker) {
        m_llmWorker->deleteLater();  // ✅ 一定先 delete Worker
        m_llmWorker = nullptr;
    }
    if (m_llmThread) {
        m_llmThread->quit();
        m_llmThread->wait();
        m_llmThread->deleteLater();  // ✅ 避免裸 delete
        m_llmThread = nullptr;
    }
}


void Widget::startDetectionPipeline()
{
    if (m_worker) {
        //qDebug() << "开始采集流程";
        QMetaObject::invokeMethod(m_worker, "process", Qt::QueuedConnection);
    }
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->inferLabel) {
        if (event->type() == QEvent::TouchEnd) {
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            for (const QTouchEvent::TouchPoint &point : touchEvent->touchPoints()) {
                QPoint labelPos = point.pos().toPoint();
                qDebug() << "Touch on inferLabel at" << labelPos;
                m_detector->handleTouch(labelPos.x(), labelPos.y());
                break;
            }
            return true;  // 只拦截inferLabel的触摸，不影响其他控件
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint labelPos = mouseEvent->pos();
            qDebug() << "Mouse click on inferLabel at" << labelPos;
            m_detector->handleTouch(labelPos.x(), labelPos.y());
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);  // 其他事件默认处理
}

void Widget::onSerialError(const QString &error)
{
    qDebug() << "Serial port error:" << error;
}

void Widget::onPortOpened()
{
    qDebug() << "Serial port opened successfully.";
}

void Widget::onWhisperResultReady(const QString &result)
{
    QString cleaned = result.trimmed();
    if (cleaned.isEmpty())
        return;

    qDebug() << "[Whisper] 识别结果：" << cleaned;

    // 原有逻辑：只在按钮按住时缓存一条
    if (m_llmActive) {
        m_lastWhisperText = cleaned;
        qDebug() << "[Whisper] 缓存到 m_lastWhisperText：" << m_lastWhisperText;
    }

    // ✅ 新增逻辑：无论是否按住，都追加到历史缓存
    m_whisperTextBuffer.append(cleaned);

    // MQTT 原始推送逻辑保留
    if (m_mqttClient && m_mqttClient->state() == QMqttClient::Connected) {
        m_mqttClient->publish(QMqttTopicName("whisper/result"), cleaned.toUtf8());
        qDebug() << "[MQTT] 已发送 Whisper 原始文本：" << cleaned;
    }
}





void Widget::initInferStreamer()
{
    // 推理图像推流地址（不同于原始摄像头流）
    std::string infer_pipeline =
        "appsrc ! "
        "videoconvert ! "
        "video/x-raw,format=I420,framerate=15/1,width=1280,height=720 ! "
        "mpph264enc ! h264parse ! "
        "flvmux ! "
//        "rtmpsink location=rtmp://192.168.10.100/live/infer";

//    "rtmpsink location=rtmp://192.168.10.200/teacher";
            "rtmpsink location=rtmp://192.168.184.52/teacher";

    // 与推理图像分辨率一致
    int width = 1280;
    int height = 720;
    double fps = 15.0;

    m_inferWriterOpened = m_inferWriter.open(infer_pipeline, 0, fps, cv::Size(width, height), true);

    if (m_inferWriterOpened) {
        qDebug() << "[推理推流] 成功打开推流器到 rtmp://192.168.141.52/teacher";
    } else {
        qWarning() << "[推理推流] 打开推流器失败，请检查 GStreamer 管道和地址";
    }
}

void Widget::onCancelTrackingClicked()
{
    if (m_detector) {
        m_detector->stopTracking();
        qDebug() << "Tracking manually cancelled.";
    }
}


void Widget::logToFile(const QString &message)
{
    QFile logFile("tracking.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ")
            << message << "\n";
        logFile.close();
    }
}

void Widget::on_btnllm_pressed()
{
    m_llmActive = true;
    qDebug() << "[LLM] 按钮按下，准备监听最新 Whisper 结果";
    // 不要清空 m_lastWhisperText！
}

void Widget::on_btnllm_released()
{
    m_llmActive = false;
    qDebug() << "[LLM按钮] 松开调用，缓存文本：" << m_lastWhisperText;

    if (m_lastWhisperText.trimmed().isEmpty()) {
        qWarning() << "[LLM] 无可用文本，跳过调用";
        return;
    }

    QString prompt = "请理解并直接回答以下中文问题，不要重复问题内容，也不要加说明：\n" + m_lastWhisperText;

    QMetaObject::invokeMethod(m_llmWorker, "runPrompt", Qt::QueuedConnection, Q_ARG(QString, prompt));

    qDebug() << "[LLM] 调用 LLM，内容：" << m_lastWhisperText;
}

void Widget::onLlmResultReady(const QString &text)
{
    QString cleaned = text.trimmed();

    // 简单过滤无效内容
    if (cleaned.isEmpty() || cleaned == "<think></think>") {
        qDebug() << "[LLM] 空回复，忽略";
        return;
    }

    qDebug() << "[LLM 回复] " << cleaned;

    // 发送到 MQTT，比如发到主题 "llm/response"
    if (m_mqttClient && m_mqttClient->state() == QMqttClient::Connected) {
        QByteArray payload = cleaned.toUtf8();
        m_mqttClient->publish(QMqttTopicName("LLM/result"), payload);
        qDebug() << "[MQTT] 已发送 LLM 回复：" << cleaned;
    }

    // 如果需要，还可以更新 UI
    // ui->llmResponseLabel->setText(cleaned);
}


void Widget::on_exitbt_clicked()
{
    qApp->quit();  // 退出整个应用

}
void Widget::onSummarizeButtonClicked()
{
    if (m_whisperTextBuffer.isEmpty()) {
        qDebug() << "[摘要按钮] 没有识别内容可以总结";
        return;
    }

    QString allText = m_whisperTextBuffer.join(" ");
    m_whisperTextBuffer.clear();  // 清空缓存

    qDebug() << "[摘要按钮] 发送文本到 LLM 总结：" << allText;

    emit sendTextToLLM(allText);  // 假设你已经连接到了 LLM 输入通道
}
//void Widget::onSummarizeButtonClicked()
//{
//    if (m_whisperTextBuffer.isEmpty()) {
//        qDebug() << "[摘要按钮] 没有识别内容可以总结";
//        return;
//    }

//    // 拼接所有语音识别文本
//    QString allText = m_whisperTextBuffer.join(" ");
//    m_whisperTextBuffer.clear();  // 清空缓存

//    QString prompt = QStringLiteral(
//        "请理解并直接回答以下中文问题，不要重复问题内容，也不要加说明：\n"
//    ).arg(allText);


//    qDebug() << "[摘要按钮] 构造后的 Prompt：" << prompt.left(200) << "...";

//    emit sendTextToLLM(prompt);  // 发给 LLMWorker
//}


