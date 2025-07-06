#include "detectorpool.h"

DetectorPool::DetectorPool(QObject* parent) : QObject(parent) {}

DetectorPool::~DetectorPool() {
    for (auto& d : m_detectors) {
        d.thread->quit();
        d.thread->wait();
        delete d.detector;
    }
}

void DetectorPool::addDetector(const QString& modelPath, int npuIndex) {
    QThread* thread = new QThread(this);
    YOLOv5Detector* detector = new YOLOv5Detector();
    detector->moveToThread(thread);

    // 连接初始化
    connect(thread, &QThread::started, [=]() {
        QMetaObject::invokeMethod(detector, "initialize", Qt::QueuedConnection,
                                  Q_ARG(QString, modelPath),
                                  Q_ARG(int, npuIndex));
    });

    // 启动线程
    thread->start();

    // 存储
    m_detectors.append({thread, detector});
}

void DetectorPool::submitFrame(const QImage& frame) {
    if (m_detectors.isEmpty())
        return;

    // 轮询分发（也可以按负载、优先级等策略分发）
    auto& detector = m_detectors[m_nextIndex].detector;
    QMetaObject::invokeMethod(detector, "detectQImage", Qt::QueuedConnection,
                              Q_ARG(QImage, frame));

    m_nextIndex = (m_nextIndex + 1) % m_detectors.size();
}

