#include "micrecorder.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <poll.h>


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
    const int buffer_frames = rate * 2; // 3秒

    snd_pcm_t *handle;
    const char *device = "hw:1,0";  // 根据 arecord -l 输出
    int err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        qWarning() << "[MicRecorder] snd_pcm_open failed:" << snd_strerror(err);
        return;
    }
    qDebug() << "[MicRecorder] snd_pcm_open 成功，设备:" << device;
    setMicVolume("Capture", 80);  // 设置“Capture”通道的录音音量为 80%

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

        // 创建单通道 float PCM
        QVector<float> pcmMono;
        pcmMono.reserve(frames_read);  // 单声道数据长度 = 帧数

        int16_t* samples = reinterpret_cast<int16_t*>(buffer);
        for (int i = 0; i < frames_read; ++i) {
            // 获取左右声道样本
            int16_t left = samples[i * 2];
            int16_t right = samples[i * 2 + 1];

            // Downmix 成 mono 并归一化到 [-1.0, 1.0]
            float mono = (left + right) / (2.0f * 32768.0f);
            pcmMono.append(mono);
        }

        qDebug() << "[MicRecorder] 发出 audioSegmentReady，当前线程：" << QThread::currentThread();
        emit audioSegmentReady(pcmMono);
        qDebug() << "[MicRecorder] 发出 audioSegmentReady，帧数:" << pcmMono.size();
    }


    delete[] buffer;

    // 采集完成后关闭设备
    snd_pcm_close(handle);
}




void MicRecorder::stop()
{
    m_running = false;
}

void MicRecorder::setMicVolume(const char *selem_name, long volume)
{
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "hw:1";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);

    snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);
    if (elem) {
        long minv, maxv;
        snd_mixer_selem_get_capture_volume_range(elem, &minv, &maxv);
        long setv = volume * (maxv - minv) / 100 + minv;
        snd_mixer_selem_set_capture_volume_all(elem, setv);
    }

    snd_mixer_close(handle);
    snd_mixer_selem_id_free(sid);
}




