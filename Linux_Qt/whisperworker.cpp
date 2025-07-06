#include "whisperworker.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QEventLoop>

WhisperWorker::WhisperWorker(QObject* parent)
    : QObject(nullptr), m_thread(new QThread)
{
    qDebug() << "[WhisperWorker] 初始化中...";

    bool initSuccess = whisper.initialize("/home/elf/model/whisper_encoder_base_20s.rknn",
                                          "/home/elf/model/whisper_decoder_base_20s.rknn");
    if (!initSuccess) {
        qWarning() << "[WhisperWorker] 模型初始化失败！";
    } else {
        qDebug() << "[WhisperWorker] 模型初始化完成";
    }

    if (!whisper.loadResources()) {
        qWarning() << "[WhisperWorker] 资源加载失败！";
    } else {
        qDebug() << "[WhisperWorker] 资源加载完成";
    }


}

WhisperWorker::~WhisperWorker()
{
    qDebug() << "[WhisperWorker] 清理中...";

    // 发出退出事件循环的信号
    emit finished();

    m_audioQueue.wakeAll();  // 唤醒等待队列

    whisper.cleanup();
}



void WhisperWorker::pushAudioData(const QVector<float>& pcmData) {
    qDebug() << "[WhisperWorker] pushAudioData called, size =" << pcmData.size()
             << "线程：" << QThread::currentThread();

    m_audioQueue.push(pcmData);
}



void WhisperWorker::processLoop()
{
    qDebug() << "[WhisperWorker] 推理线程启动";

    // 定时器每 100ms 检查一次音频队列
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        QVector<float> pcmData;
        if (m_audioQueue.waitAndPop(pcmData, 0)) {
            qDebug() << "[WhisperWorker] 收到音频段：" << pcmData.size()
                     << "线程：" << QThread::currentThread();

            std::vector<float> pcmStd(pcmData.begin(), pcmData.end());
            std::vector<std::string> output;

            if (whisper.transcribeFromPcm(pcmStd, output)) {
                QString result;
                for (const auto& s : output)
                    if (s.empty()) {
                            // 如果字符串空，则拼接空 QString
                            result += QString();
                        } else {
                            result += QString::fromStdString(s);
                        }
                emit resultReady(result);
            } else {
                emit resultReady("Whisper inference failed");
            }
        }
    });
    timer->start(100); // 每 100ms 检查队列

    // 进入事件循环，保证槽函数可以被调用
    QEventLoop loop;
    connect(this, &WhisperWorker::finished, &loop, &QEventLoop::quit);
    loop.exec();

    qDebug() << "[WhisperWorker] 推理线程退出";
}

void WhisperWorker::stop() {
    qDebug() << "[WhisperWorker] stop() 被调用";
    m_running = false;
}



