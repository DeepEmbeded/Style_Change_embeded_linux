#include "whisper_rknn.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

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

bool whisper_rknn::transcribeFromPcm(const std::vector<float>& pcmData, std::vector<std::string>& outputText) {
    if (!initialized || pcmData.empty()) {
        std::cerr << "[Whisper] Not initialized or empty PCM data" << std::endl;
        return false;
    }

    audio_buffer_t audio;
    memset(&audio, 0, sizeof(audio));

    audio.sample_rate = SAMPLE_RATE;
    audio.num_frames = static_cast<int>(pcmData.size());
    audio.data = static_cast<float*>(malloc(sizeof(float) * audio.num_frames));
    if (!audio.data) {
        std::cerr << "[Whisper] Failed to allocate audio buffer" << std::endl;
        return false;
    }

    memcpy(audio.data, pcmData.data(), sizeof(float) * audio.num_frames);

    int ret = inference_whisper_model(&rknn_ctx, &audio, mel_filters, vocab, outputText);

    free(audio.data);
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
