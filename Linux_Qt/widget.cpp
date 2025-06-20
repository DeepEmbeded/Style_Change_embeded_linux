#include "widget.h"
#include "ui_widget.h"
#include "worker.h"
#include "yolov5detector.h"
#include <QDebug>
#include <QTimer>
#include <QTouchEvent>

const float HFOV = 70.0f; // 水平视场角（度）
const float VFOV = 65.0f; // 垂直视场角（度）

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
  , m_workerThread(nullptr)
  , m_detectorThread(nullptr)
  , m_worker(nullptr)
  , m_detector(nullptr)
  , micThread(nullptr)
  , mic(nullptr)
{
    ui->setupUi(this);

    // 全屏显示
    this->showFullScreen();

    ui->inferLabel->setAttribute(Qt::WA_AcceptTouchEvents);  // 支持触摸事件
    ui->inferLabel->installEventFilter(this);                // 安装事件过滤器


    // 初始化UI
    ui->inferLabel->setAlignment(Qt::AlignCenter);

    ui->inferLabel->setText("等待启动...");
    //ui->videoLabel->setAlignment(Qt::AlignCenter);

    //新增：串口
    m_serialManager = new SerialPortManager(this);
    connect(m_serialManager, &SerialPortManager::errorOccurred, this, &Widget::onSerialError);
    connect(m_serialManager, &SerialPortManager::portOpened, this, &Widget::onPortOpened);

    // 例如打开串口 /dev/ttyUSB0，波特率115200
    m_serialManager->openPort("/dev/ttyS9", QSerialPort::Baud115200);


    // 按钮连接
    connect(ui->btnStart, &QPushButton::clicked, this, &Widget::onStartClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &Widget::onStopClicked);

    //新增：mqtt
    mqttReceiver = new MqttReceiver(this);
    mqttReceiver->connectToBroker("192.168.10.100", 1883); // PC的IP
    qDebug() << "[MQTT] 正在连接至 Broker: 192.168.10.100:1883";

    //实例化 MQTT 客户端
    m_mqttClient = new QMqttClient(this);
    m_mqttClient->setHostname("192.168.10.100");  // 替换为你的 MQTT Broker IP
    m_mqttClient->setPort(1883);                // 默认端口
    m_mqttClient->connectToHost();

   connect(m_mqttClient, &QMqttClient::connected, this, [] {
      qDebug() << "[MQTT] 已连接到 Broker";
   });

   connect(m_mqttClient, &QMqttClient::disconnected, this, [] {
      qDebug() << "[MQTT] 与 Broker 断开连接";
   });


    //新增：FPS
    connect(m_detector, &YOLOv5Detector::fpsUpdated, this, [=](double fps){
        ui->inferLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 2));
    });


    // 初始化线程
    initThreads();
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
    // 初始化检测器模型
    //qDebug() << "收到推理结果，更新inferLabel";
    QPixmap pixmap = QPixmap::fromImage(result)
                    .scaled(ui->inferLabel->size(), Qt::KeepAspectRatio);

    // 获取显示分辨率（缩放后的实际尺寸）
        int display_width = pixmap.width();   // 显示宽度
        int display_height = pixmap.height(); // 显示高度

        qDebug() << "显示分辨率：" << display_width << "x" << display_height;



    ui->inferLabel->setPixmap(pixmap);

    //新增：推
    //转换为 cv::Mat（BGR888）
    QImage img = result.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(img.height(), img.width(), CV_8UC3,
                const_cast<uchar*>(img.bits()), img.bytesPerLine());

    // OpenCV 默认是 BGR，转换格式
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);

    // 推流
    if (m_inferWriterOpened) {
        m_inferWriter.write(bgr);
    } else {
        qWarning() << "[推理推流] 推流器未打开，帧未写入";
    }

    // 可选：打印尺寸
    qDebug() << "推理帧尺寸：" << img.width() << "x" << img.height();
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

void Widget::onStopClicked()
{
    // 初始化检测器模型
    //qDebug() << "[MainWindow] 初始化检测器模型...";
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);
    }
    cleanup();
}

void Widget::onWorkerFinished()
{
    // 初始化检测器模型
    //qDebug() << "[MainWindow] 初始化检测器模型...";
}

void Widget::initThreads()
{
    // 视频采集线程
    m_workerThread = new QThread(this);
    m_worker = new Worker();
    m_worker->moveToThread(m_workerThread);
    connect(m_worker, &Worker::frameReady, this, &Widget::updateFrame);
    connect(m_worker, &Worker::finished, this, &Widget::onWorkerFinished);

    // 检测器线程
    m_detectorThread = new QThread(this);
    m_detector = new YOLOv5Detector();
    m_detector->moveToThread(m_detectorThread);
    connect(m_detector, &YOLOv5Detector::detectionComplete, this, &Widget::updateInferResult);
    connect(m_detector, &YOLOv5Detector::errorOccurred, this, &Widget::handleError);

    // 音频采集线程
    micThread = new QThread(this);
    mic = new MicRecorder();
    mic->moveToThread(micThread);
    connect(micThread, &QThread::finished, mic, &QObject::deleteLater);
    connect(micThread, &QThread::started, mic, &MicRecorder::start);

    whisperThread = new QThread(this);                     // 外部线程
    whisperworker = new WhisperWorker();                   // 不 moveToThread 自己

    whisperworker->moveToThread(whisperThread);            // 由外部移动

    connect(whisperThread, &QThread::started, whisperworker, &WhisperWorker::processLoop);
    connect(whisperThread, &QThread::finished, whisperworker, &QObject::deleteLater);

    // 音频采集 -> 推理输入
//    connect(mic, &MicRecorder::audioSegmentReady,
//            whisperworker, &WhisperWorker::pushAudioData,
//            Qt::QueuedConnection);
    bool ok = connect(mic, &MicRecorder::audioSegmentReady,
                      whisperworker, &WhisperWorker::pushAudioData,
                      Qt::QueuedConnection);
    qDebug() << "connect audioSegmentReady to pushAudioData result:" << ok;



    // 推理结果 -> 界面更新
    connect(whisperworker, &WhisperWorker::resultReady,
            this, &Widget::onWhisperResultReady);

    // 启动线程
    m_workerThread->start();
    m_detectorThread->start();
    micThread->start();
    whisperThread->start();


    initInferStreamer();  // ✅ 初始化推理推流器
    // 初始化检测器模型
    QMetaObject::invokeMethod(m_detector, "initialize", Qt::QueuedConnection,
                              Q_ARG(QString, "model/RK3588/yolov5s-640-640.rknn"),
                              Q_ARG(int, 0));

    // MQTT 触发目标追踪
    connect(mqttReceiver, &MqttReceiver::coordinateReceived,
            m_detector, &YOLOv5Detector::handleTouch);

    // 坐标输出到串口（中心角度）
    connect(m_detector, &YOLOv5Detector::trackingUpdated, this, [=](const QRect &rect) {
        int frame_width = ui->inferLabel->width();
        int frame_height = ui->inferLabel->height();

        float cx = rect.x() + rect.width() / 2.0f;
        float cy = rect.y() + rect.height() / 2.0f;
        float dx = cx - frame_width / 2.0f;
        float dy = cy - frame_height / 2.0f;

        float angle_x = dx / frame_width * HFOV;
        float angle_y = dy / frame_height * VFOV;

        QString data = QString("AX:%1 AY:%2\n")
                           .arg(QString::number(angle_x, 'f', 2))
                           .arg(QString::number(angle_y, 'f', 2));
        m_serialManager->sendData(data.toUtf8());
    });
}


void Widget::cleanup()
{
    qDebug() << "清理资源";
    // 停止 Whisper 推理线程
        if (whisperworker) {
//            whisperworker->stop();
            delete whisperworker;
            whisperworker = nullptr;
        }

        // 停止音频线程
        if (micThread) {
            micThread->quit();
            micThread->wait();
            delete micThread;
        }

        // 停止检测线程等
        if (m_detectorThread) {
            m_detectorThread->quit();
            m_detectorThread->wait();
            delete m_detectorThread;
        }

        if (m_workerThread) {
            m_workerThread->quit();
            m_workerThread->wait();
            delete m_workerThread;
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
    qDebug() << "Whisper识别结果：" << result;

    // ✅ MQTT 推送文本
    if (m_mqttClient && m_mqttClient->state() == QMqttClient::Connected) {
        m_mqttClient->publish(QMqttTopicName("whisper/result"), result.toUtf8());
        qDebug() << "[MQTT] 已发送识别文本";
    } else {
        qWarning() << "[MQTT] 尚未连接，无法发送识别结果";
    }

    // ✅ 可选：更新 UI
    // ui->textEdit->append(result);
}

void Widget::initInferStreamer()
{
    // 推理图像推流地址（不同于原始摄像头流）
    std::string infer_pipeline =
        "appsrc ! "
        "videoconvert ! "
        "video/x-raw,format=I420 ! "
        "mpph264enc ! h264parse ! "
        "flvmux ! "
        "rtmpsink location=rtmp://192.168.10.50/live/infer";

    // 与推理图像分辨率一致
    int width = 1280;
    int height = 720;
    double fps = 15.0;

    m_inferWriterOpened = m_inferWriter.open(infer_pipeline, 0, fps, cv::Size(width, height), true);

    if (m_inferWriterOpened) {
        qDebug() << "[推理推流] 成功打开推流器到 rtmp://192.168.10.51/live/infer";
    } else {
        qWarning() << "[推理推流] 打开推流器失败，请检查 GStreamer 管道和地址";
    }
}

