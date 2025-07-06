#ifndef LLMWORKER_H
#define LLMWORKER_H

#pragma once

#include <QObject>
#include <QString>
#include <QMutex>
#include <QQueue>

#include "rkllm.h"

class LLMWorker : public QObject
{
    Q_OBJECT
public:
    explicit LLMWorker(QObject *parent = nullptr);
    ~LLMWorker();

    void setModelPath(const QString &path);
    void setTokenParams(int maxNewTokens, int maxContextLen);

public slots:
    void init();
    void deinit();
    void runPrompt(const QString &prompt);  // 触发推理请求
    void clearCache();
    void stop();  // ✅ 添加这一行

signals:
    void llmResultReady(const QString &text);
    void initialized(bool success);

private:
    static void staticCallback(RKLLMResult *result, void *userdata, LLMCallState state);
    void handleCallback(RKLLMResult *result, LLMCallState state);

private:
    QString m_modelPath;
    int m_maxNewTokens = 64;
    int m_maxContextLen = 512;

    RKLLMInferParam m_inferParam;
    LLMHandle m_handle = nullptr;

    QString m_fullText;
};


#endif // LLMWORKER_H
