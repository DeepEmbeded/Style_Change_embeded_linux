#include "micrecorder.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <poll.h>
#include <webrtc_vad.h>



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



void MicRecorder::start()
{
    qDebug() << "[MicRecorder] ALSA 初始化启动，线程 ID:" << QThread::currentThreadId();

    const unsigned int rate = 16000;
    const snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    const int channels = 2;
    const int frame_duration_ms = 10;
    const int frame_samples = rate * frame_duration_ms / 1000;  // 160 samples
    const int buffer_frames = frame_samples;

    // 初始化 WebRTC VAD
    VadInst* vad = WebRtcVad_Create();
    if (!vad || WebRtcVad_Init(vad) != 0 || WebRtcVad_set_mode(vad, 2) != 0) {
        qWarning() << "[MicRecorder] WebRTC VAD 初始化失败";
        if (vad) WebRtcVad_Free(vad);
        return;
    }

    snd_pcm_t* handle;
    const char* device = "hw:1,0";
    int err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        qWarning() << "[MicRecorder] snd_pcm_open failed:" << snd_strerror(err);
        WebRtcVad_Free(vad);
        return;
    }

    setMicVolume("Capture", 100);

    snd_pcm_hw_params_t* params = nullptr;
    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, format);
    snd_pcm_hw_params_set_channels(handle, params, channels);
    snd_pcm_hw_params_set_rate(handle, params, rate, 0);
    snd_pcm_hw_params(handle, params);
    snd_pcm_hw_params_free(params);
    snd_pcm_prepare(handle);

    qDebug() << "[MicRecorder] ALSA 初始化完成，可开始采集。";

    int frame_size = snd_pcm_format_width(format) / 8 * channels;
    int buffer_size = buffer_frames * frame_size;
    char* buffer = new char[buffer_size];

    QVector<float> speechSegment;
    qint64 lastSpeechTimestamp = 0;
    const int silenceTimeoutMs = 800;
    const int maxSegmentSamples = rate * 10;  // 10秒最大语音段
    const int minSegmentSamples = rate * 0.5; // 至少0.5秒语音段

    m_running = true;

    while (m_running) {
        int frames_read = snd_pcm_readi(handle, buffer, buffer_frames);
        if (frames_read == -EPIPE) {
            snd_pcm_prepare(handle);
            qWarning() << "[MicRecorder] 过载，重置 PCM";
            continue;
        } else if (frames_read < 0) {
            snd_pcm_prepare(handle);
            qWarning() << "[MicRecorder] 读取失败:" << snd_strerror(frames_read);
            continue;
        } else if (frames_read != buffer_frames) {
            qWarning() << "[MicRecorder] 读取帧数不完整:" << frames_read << "/" << buffer_frames;
        }

        int16_t* samples = reinterpret_cast<int16_t*>(buffer);

        // Downmix to mono int16_t for VAD
        std::vector<int16_t> monoInt16(frame_samples);
        QVector<float> monoFloat;
        monoFloat.reserve(frame_samples);

        for (int i = 0; i < frames_read; ++i) {
            int16_t left = samples[i * 2];
            int16_t right = samples[i * 2 + 1];
            int16_t mono = (left + right) / 2;
            monoInt16[i] = mono;
            monoFloat.append(mono / 32768.0f);  // float [-1.0, 1.0]
        }

        // VAD 判断
        int vad_ret = WebRtcVad_Process(vad, rate, monoInt16.data(), frame_samples);
        bool isSpeech = (vad_ret == 1);
        qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

        if (isSpeech) {
            speechSegment += monoFloat;
            lastSpeechTimestamp = nowMs;
//            qDebug() << "[MicRecorder] 检测到语音，累积帧数:" << speechSegment.size();
        } else {
            if (!speechSegment.isEmpty() && (nowMs - lastSpeechTimestamp > silenceTimeoutMs)) {
                if (speechSegment.size() >= minSegmentSamples) {
//                    qDebug() << "[MicRecorder] 语音段结束，发出信号，帧数:" << speechSegment.size();
                    emit audioSegmentReady(speechSegment);
                } else {
//                    qDebug() << "[MicRecorder] 丢弃短语音段，帧数:" << speechSegment.size();
                }
                speechSegment.clear();
            }
        }

        if (speechSegment.size() > maxSegmentSamples) {
//            qDebug() << "[MicRecorder] 语音段过长，强制发出，帧数:" << speechSegment.size();
            emit audioSegmentReady(speechSegment);
            speechSegment.clear();
        }

        QThread::msleep(frame_duration_ms);

    }
//    while (m_running) {
//        int frames_read = snd_pcm_readi(handle, buffer, buffer_frames);
//        if (frames_read == -EPIPE) {
//            snd_pcm_prepare(handle);
//            qWarning() << "[MicRecorder] 过载，重置 PCM";
//            continue;
//        } else if (frames_read < 0) {
//            snd_pcm_prepare(handle);
//            qWarning() << "[MicRecorder] 读取失败:" << snd_strerror(frames_read);
//            continue;
//        } else if (frames_read != buffer_frames) {
//            qWarning() << "[MicRecorder] 读取帧数不完整:" << frames_read << "/" << buffer_frames;
//        }

//        int16_t* samples = reinterpret_cast<int16_t*>(buffer);

//        std::vector<int16_t> monoInt16(frame_samples);
//        QVector<float> monoFloat;
//        monoFloat.reserve(frame_samples);

//        float peak = 0.0f;  // ⚡ 添加能量峰值变量

//        for (int i = 0; i < frames_read; ++i) {
//            int16_t left = samples[i * 2];
//            int16_t right = samples[i * 2 + 1];
//            int16_t mono = (left + right) / 2;
//            monoInt16[i] = mono;
//            float normalized = mono / 32768.0f;
//            monoFloat.append(normalized);
//            peak = std::max(peak, std::abs(normalized));  // ⚡ 实时计算最大峰值
//        }

//        // VAD 判断
//        int vad_ret = WebRtcVad_Process(vad, rate, monoInt16.data(), frame_samples);
//        bool isVadSpeech = (vad_ret == 1);
//        bool isLikelySpeech = isVadSpeech || (peak > 0.02f);  // ⚡ 辅助判定规则
//        if (isVadSpeech) {
//            if (peak > 0.005f) {  // ✨ 只有有足够能量才信 VAD 判语音
//                isLikelySpeech = true;
//            } else {
//                qDebug() << "[滤除] VAD虽为语音但能量太低，疑似误判 | Peak:" << peak;
//            }
//        } else if (peak > 0.02f) {  // ✨ 语音能量强但 VAD 不判语音，也信
//            isLikelySpeech = true;
//        }
//        qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

//        if (isLikelySpeech) {
//            speechSegment += monoFloat;
//            lastSpeechTimestamp = nowMs;

//            qDebug() << "[MicRecorder] ✅ 检测到语音 | VAD:" << vad_ret
//                     << "| Peak:" << peak
//                     << "| 累积帧数:" << speechSegment.size();
//        } else {
//            if (!speechSegment.isEmpty() && (nowMs - lastSpeechTimestamp > silenceTimeoutMs)) {
//                if (speechSegment.size() >= minSegmentSamples) {
//                    qDebug() << "[MicRecorder] ✅ 语音段结束，发出信号，帧数:" << speechSegment.size();
//                    emit audioSegmentReady(speechSegment);
//                } else {
//                    qDebug() << "[MicRecorder] ❌ 丢弃短语音段，帧数:" << speechSegment.size();
//                }
//                speechSegment.clear();
//            }
//        }

//        if (speechSegment.size() > maxSegmentSamples) {
//            qDebug() << "[MicRecorder] ⚠️ 语音段过长，强制发出，帧数:" << speechSegment.size();
//            emit audioSegmentReady(speechSegment);
//            speechSegment.clear();
//        }

//        QThread::msleep(frame_duration_ms);
//    }


    delete[] buffer;
    snd_pcm_close(handle);
    WebRtcVad_Free(vad);

    qDebug() << "[MicRecorder] 停止采集";
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




