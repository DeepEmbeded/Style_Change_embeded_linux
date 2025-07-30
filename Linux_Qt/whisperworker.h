#ifndef WHISPERWORKER_H
#define WHISPERWORKER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "whisper_rknn.h"
#include <QVector>
#include "safequeue.h"
#include <QThread>
//#include <QAtomicBool>



class QThread;

class WhisperWorker : public QObject
{
    Q_OBJECT
public:
    explicit WhisperWorker(QObject* parent = nullptr);
    ~WhisperWorker();

    // 供外部调用，往队列推数据


public slots:
    void processLoop();  // 让其成为 Qt 槽函数以便连接
    void pushAudioData(const QVector<float>& pcmData);
    void stop();
signals:
    void resultReady(const QString& result);
    void finished();

private:
    SafeQueue<QVector<float>> m_audioQueue;
    QThread* m_thread = nullptr;
    QMutex m_queueMutex;
    bool m_running = false;
//    QAtomicBool m_running { false };


    whisper_rknn whisper;


};

#endif // WHISPERWORKER_H
