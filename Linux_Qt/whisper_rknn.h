#ifndef WHISPER_RKNN_H
#define WHISPER_RKNN_H

#include <vector>
#include <string>
#include "whisper.h"
#include "audio_utils.h"

class whisper_rknn
{
public:
    whisper_rknn();
    ~whisper_rknn();

    bool initialize(const std::string& encoderPath, const std::string& decoderPath);
    bool loadResources(const std::string& melFilterPath = MEL_FILTERS_PATH,
                       const std::string& vocabPath = VOCAB_PATH);
    bool processAudioFile(const std::string& audioPath, int taskCode = 50260);
    bool transcribeFromPcm(const std::vector<float>& pcmData, std::vector<std::string>& outputText);
    void cleanup();

private:
    rknn_whisper_context_t rknn_ctx;
    float* mel_filters = nullptr;
    VocabEntry vocab[VOCAB_NUM];
    bool initialized = false;
};

#endif // WHISPER_RKNN_H
