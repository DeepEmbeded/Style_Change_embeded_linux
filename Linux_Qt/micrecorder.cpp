#include "micrecorder.h"
#include <alsa/asoundlib.h>
#include <QDateTime>
#include <QDebug>
#include <QFile>

MicRecorder::MicRecorder(QObject *parent) : QObject(parent)
{
    qDebug() << "[MicRecorder] 构造";

}

MicRecorder::~MicRecorder()
{
    stop();
}

void MicRecorder::setDurationSec(int seconds)
{
    m_recordDurationSec = seconds;
}

// micrecorder.cpp

void MicRecorder::start()
{
    qDebug() << "[MicRecorder] ALSA 初始化启动，线程 ID:" << QThread::currentThreadId();

    const unsigned int rate = 16000;
    const snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    const int channels = 2;
    const int buffer_frames = rate * 3; // 3秒

    snd_pcm_t *handle;
    const char *device = "hw:1,0";  // 根据 arecord -l 输出
    int err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        qWarning() << "[MicRecorder] snd_pcm_open failed:" << snd_strerror(err);
        return;
    }
    qDebug() << "[MicRecorder] snd_pcm_open 成功，设备:" << device;

    snd_pcm_hw_params_t *params = nullptr;
    err = snd_pcm_hw_params_malloc(&params);
    if (err < 0 || params == nullptr) {
        qWarning() << "[MicRecorder] snd_pcm_hw_params_malloc failed";
        snd_pcm_close(handle);
        return;
    }

    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        qWarning() << "[MicRecorder] snd_pcm_hw_params_any failed:" << snd_strerror(err);
        snd_pcm_hw_params_free(params);
        snd_pcm_close(handle);
        return;
    }

    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        qWarning() << "[ALSA] set_access 失败:" << snd_strerror(err);
        snd_pcm_hw_params_free(params);
        snd_pcm_close(handle);
        return;
    }

    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
        qWarning() << "[ALSA] set_format 失败:" << snd_strerror(err);
        snd_pcm_hw_params_free(params);
        snd_pcm_close(handle);
        return;
    }

    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0) {
        qWarning() << "[ALSA] set_channels 失败:" << snd_strerror(err);
        snd_pcm_hw_params_free(params);
        snd_pcm_close(handle);
        return;
    }

    err = snd_pcm_hw_params_set_rate(handle, params, rate, 0);
    if (err < 0) {
        qWarning() << "[ALSA] set_rate 失败:" << snd_strerror(err);
        snd_pcm_hw_params_free(params);
        snd_pcm_close(handle);
        return;
    }

    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        qWarning() << "[ALSA] hw_params 设置失败:" << snd_strerror(err);
        snd_pcm_hw_params_free(params);
        snd_pcm_close(handle);
        return;
    }

    snd_pcm_hw_params_free(params);

    err = snd_pcm_prepare(handle);
    if (err < 0) {
        qWarning() << "[ALSA] snd_pcm_prepare 失败:" << snd_strerror(err);
        snd_pcm_close(handle);
        return;
    }

    qDebug() << "[MicRecorder] ALSA 初始化完成，可开始采集。";

    // 这里写采集代码，例如采集循环
    // 计算缓冲区大小
    int frame_size = snd_pcm_format_width(format) / 8 * channels;
    int buffer_size = buffer_frames * frame_size;
    char* buffer = new char[buffer_size];

    m_running = true;

    while (m_running) {
        int frames_read = snd_pcm_readi(handle, buffer, buffer_frames);
        if (frames_read == -EPIPE) {
            // 过载，重启采集
            snd_pcm_prepare(handle);
            qWarning() << "[MicRecorder] snd_pcm_readi failed: overrun, reset PCM";
            continue;
        } else if (frames_read < 0) {
            snd_pcm_prepare(handle);
            qWarning() << "[MicRecorder] snd_pcm_readi failed:" << snd_strerror(frames_read);
            continue;
        } else if (frames_read != buffer_frames) {
            qWarning() << "[MicRecorder] snd_pcm_readi 读取帧数不完整:" << frames_read << "/" << buffer_frames;
        }

        QVector<float> pcm;
        pcm.reserve(frames_read * channels);

        int16_t* samples = reinterpret_cast<int16_t*>(buffer);
        for (int i = 0; i < frames_read * channels; ++i) {
            pcm.append(samples[i] / 32768.0f);  // 归一化到 [-1, 1]
        }
        qDebug() << "[MicRecorder] 发出 audioSegmentReady，当前线程：" << QThread::currentThread();

        emit audioSegmentReady(pcm);

        qDebug() << "[MicRecorder] 发出 audioSegmentReady，帧数:" << pcm.size();
    }

    delete[] buffer;

    // 采集完成后关闭设备
    snd_pcm_close(handle);
}




void MicRecorder::stop()
{
    m_running = false;
}




