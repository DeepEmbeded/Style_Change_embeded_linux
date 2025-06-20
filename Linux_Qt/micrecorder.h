#ifndef MICRECORDER_H
#define MICRECORDER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <atomic>  // 替代 QAtomicBool


class MicRecorder : public QObject
{
    Q_OBJECT
public:
    explicit MicRecorder(QObject *parent = nullptr);
    ~MicRecorder();

    void setDurationSec(int seconds);
    void start();
    void stop();

signals:
    void audioSegmentReady(const QVector<float>& pcmData);



private:
    std::atomic_bool m_running {false};  // 替换 QAtomicBool
    int m_recordDurationSec = 5; // 默认每段录音时长
    QString m_deviceName = "hw:1,0";
};

#endif // MICRECORDER_H
