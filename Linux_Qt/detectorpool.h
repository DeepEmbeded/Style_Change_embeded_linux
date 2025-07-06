#ifndef DETECTORPOOL_H
#define DETECTORPOOL_H

#include <QObject>
#include "yolov5detector.h"
#include <QThread>

class DetectorPool : public QObject {
    Q_OBJECT
public:
    explicit DetectorPool(QObject* parent = nullptr);
    ~DetectorPool();

    void addDetector(const QString& modelPath, int npuIndex);
    void submitFrame(const QImage& frame);

private:
    struct DetectorInstance {
        QThread* thread;
        YOLOv5Detector* detector;
    };

    QVector<DetectorInstance> m_detectors;
    int m_nextIndex = 0;
};

#endif // DETECTORPOOL_H
