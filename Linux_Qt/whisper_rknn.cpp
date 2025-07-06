#include "whisper_rknn.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <QElapsedTimer>  // Qt 计时器头文件
#include "logger.h"

whisper_rknn::whisper_rknn() {
    memset(&rknn_ctx, 0, sizeof(rknn_ctx));
    memset(vocab, 0, sizeof(vocab));
    mel_filters = (float*)malloc(N_MELS * MELS_FILTERS_SIZE * sizeof(float));
}

whisper_rknn::~whisper_rknn() {
    cleanup();
}

bool whisper_rknn::initialize(const std::string& encoderPath, const std::string& decoderPath) {
    if (init_whisper_model(encoderPath.c_str(), &rknn_ctx.encoder_context, 1) != 0) {
        std::cerr << "[Whisper] Failed to init encoder" << std::endl;
        return false;
    }
    if (init_whisper_model(decoderPath.c_str(), &rknn_ctx.decoder_context, 2) != 0) {
        std::cerr << "[Whisper] Failed to init decoder" << std::endl;
        return false;
    }

    initialized = true;
    return true;
}

bool whisper_rknn::loadResources(const std::string& melFilterPath, const std::string& vocabPath) {
    if (!initialized) return false;

    if (read_mel_filters(melFilterPath.c_str(), mel_filters, N_MELS * MELS_FILTERS_SIZE) != 0) {
        std::cerr << "[Whisper] Failed to load mel filters" << std::endl;
        return false;
    }

    if (read_vocab(vocabPath.c_str(), vocab) != 0) {
        std::cerr << "[Whisper] Failed to load vocab" << std::endl;
        return false;
    }

    return true;
}

/*
* 需要改动：audio处理部分，其中main里头、utils里头。
*/
//bool whisper_rknn::transcribeFromPcm(const std::vector<float>& pcmData, std::vector<std::string>& outputText) {
//    if (!initialized || pcmData.empty()) {
//        std::cerr << "[Whisper] Not initialized or empty PCM data" << std::endl;
//        return false;
//    }

//    audio_buffer_t audio;
//    memset(&audio, 0, sizeof(audio));

//    audio.sample_rate = SAMPLE_RATE;
//    audio.num_frames = static_cast<int>(pcmData.size());
//    audio.data = static_cast<float*>(malloc(sizeof(float) * audio.num_frames));
//    if (!audio.data) {
//        std::cerr << "[Whisper] Failed to allocate audio buffer" << std::endl;
//        return false;
//    }

//    memcpy(audio.data, pcmData.data(), sizeof(float) * audio.num_frames);

//    int ret = inference_whisper_model(&rknn_ctx,
//                                       pcmData,
//                                       mel_filters,
//                                       vocab,
//                                       /* task_code */ 50260,
//                                       outputText);


//    free(audio.data);
//    return ret == 0;
//}
bool whisper_rknn::transcribeFromPcm(const std::vector<float>& pcmData,
                                     std::vector<std::string>& outputText)
{
    if (!initialized || pcmData.empty())
    {
        std::cerr << "[Whisper] Not initialized or empty PCM data" << std::endl;
        return false;
    }
    Logger::instance().appendLine(QString("开始转录，输入帧数: %1").arg(pcmData.size()));
    // 封装 audio_buffer_t
    audio_buffer_t audio;
    memset(&audio, 0, sizeof(audio));

    audio.sample_rate = SAMPLE_RATE;  // 默认认为输入已经是 16kHz
    audio.num_frames = static_cast<int>(pcmData.size());


    audio.data = static_cast<float*>(malloc(sizeof(float) * audio.num_frames));
    if (!audio.data)
    {
        std::cerr << "[Whisper] Failed to allocate audio buffer" << std::endl;
        return false;
    }

    memcpy(audio.data, pcmData.data(), sizeof(float) * audio.num_frames);

    // 预处理音频，生成 Mel 频谱
    std::vector<float> audio_data(N_MELS * MAX_AUDIO_LENGTH / HOP_LENGTH, 0.0f);
    audio_preprocess(&audio, mel_filters, audio_data);


    free(audio.data);  // 释放 audio buffer

    // 中文任务码（50260）
    int task_code = 50260;
    // ====== Qt推理时间测试 ======
    QElapsedTimer timer;
    timer.start();
    // 执行推理
    int ret = inference_whisper_model(&rknn_ctx,
                                      audio_data,
                                      mel_filters,
                                      vocab,
                                      task_code,
                                      outputText);
    qint64 elapsedMs = timer.elapsed();
    Logger::instance().appendLine(QString("推理耗时: %1 ms").arg(elapsedMs));
    // ===========================

    return ret == 0;
}



void whisper_rknn::cleanup() {
    release_whisper_model(&rknn_ctx.encoder_context);
    release_whisper_model(&rknn_ctx.decoder_context);

    if (mel_filters) {
        free(mel_filters);
        mel_filters = nullptr;
    }

    for (int i = 0; i < VOCAB_NUM; ++i) {
        if (vocab[i].token) {
            free(vocab[i].token);
            vocab[i].token = nullptr;
        }
    }

    initialized = false;
}
