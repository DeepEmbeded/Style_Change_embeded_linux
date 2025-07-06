#include "llmworker.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

LLMWorker::LLMWorker(QObject *parent) : QObject(parent)
{
    memset(&m_inferParam, 0, sizeof(RKLLMInferParam));
    m_inferParam.keep_history = 0;
}

LLMWorker::~LLMWorker()
{
    deinit();
}

void LLMWorker::setModelPath(const QString &path)
{
    m_modelPath = path;
}

void LLMWorker::setTokenParams(int maxNewTokens, int maxContextLen)
{
    m_maxNewTokens = maxNewTokens;
    m_maxContextLen = maxContextLen;
}

void LLMWorker::init()
{
    if (m_modelPath.isEmpty()) {
        qWarning() << "[LLM] 模型路径未设置";
        emit initialized(false);
        return;
    }

    RKLLMParam param = rkllm_createDefaultParam();
    param.model_path = m_modelPath.toUtf8().data();
    param.top_k = 1;
    param.top_p = 0.95;
    param.temperature = 0.8;
    param.repeat_penalty = 1.1;
    param.skip_special_token = true;
    param.max_new_tokens = m_maxNewTokens;
    param.max_context_len = m_maxContextLen;
    param.extend_param.base_domain_id = 0;
    param.extend_param.embed_flash = 1;

    int ret = rkllm_init(&m_handle, &param, staticCallback);
    if (ret != 0) {
        qWarning() << "[LLM] 初始化失败";
        emit initialized(false);
        return;
    }

    rkllm_set_chat_template(m_handle, "", "<｜User｜>", "<｜Assistant｜>");
    emit initialized(true);
}

void LLMWorker::deinit()
{
    if (m_handle) {
        rkllm_destroy(m_handle);
        m_handle = nullptr;
    }
}

void LLMWorker::clearCache()
{
    if (m_handle) {
        int ret = rkllm_clear_kv_cache(m_handle, 1);
        if (ret != 0) {
            qWarning() << "[LLM] 清理 cache 失败";
        }
    }
}

void LLMWorker::stop()
{
    qDebug() << "[LLMWorker] stop() 被调用";
    // 如果你有线程工作，可加标志终止
    // 或立即清理模型等
    deinit();  // 例如直接调用 deinit() 停止推理引擎
}

//void LLMWorker::runPrompt(const QString &prompt)
//{
//    if (!m_handle)
//        return;

//    RKLLMInput input;
//    input.input_type = RKLLM_INPUT_PROMPT;
//    input.prompt_input = (char *)prompt.toUtf8().constData();

//    emit llmResultReady("🤖: ");  // 启动提示

//    rkllm_run(m_handle, &input, &m_inferParam, this);  // userdata 为 this
//}
void LLMWorker::runPrompt(const QString &prompt)
{
    if (!m_handle || prompt.trimmed().isEmpty())
        return;

    QString safePrompt = prompt.trimmed();
    safePrompt.remove(QRegularExpression("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F]"));

    RKLLMInput input;
    input.input_type = RKLLM_INPUT_PROMPT;

    // ✅ 必须保留这个 QByteArray，不然 constData() 会悬空
    QByteArray utf8Prompt = safePrompt.toUtf8();
    input.prompt_input = utf8Prompt.data();

    emit llmResultReady("🤖: ");

    try {
        rkllm_run(m_handle, &input, &m_inferParam, this);
    } catch (const std::exception &e) {
        emit llmResultReady(QString("[LLM 崩溃] ") + e.what());
    } catch (...) {
        emit llmResultReady("[LLM 崩溃] 未知异常");
    }
}


void LLMWorker::staticCallback(RKLLMResult *result, void *userdata, LLMCallState state)
{
    LLMWorker *self = reinterpret_cast<LLMWorker *>(userdata);
    if (self) {
        self->handleCallback(result, state);
    }
}

//void LLMWorker::handleCallback(RKLLMResult *result, LLMCallState state)
//{
//    if (state == RKLLM_RUN_ERROR) {
//        emit llmResultReady("[LLM推理错误]");
//    } else if (state == RKLLM_RUN_FINISH) {
//        emit llmResultReady("\n");
//    } else if (state == RKLLM_RUN_NORMAL) {
//        QString reply = QString::fromUtf8(result->text);
//        emit llmResultReady(reply);

//        // 可选保存 last_hidden_layer
//        if (result->last_hidden_layer.embd_size != 0 && result->last_hidden_layer.num_tokens != 0) {
//            int size = result->last_hidden_layer.embd_size * result->last_hidden_layer.num_tokens * sizeof(float);
//            QFile file("last_hidden_layer.bin");
//            if (file.open(QIODevice::WriteOnly)) {
//                file.write((const char *)result->last_hidden_layer.hidden_states, size);
//                file.close();
//            }
//        }
//    }
//}
void LLMWorker::handleCallback(RKLLMResult *result, LLMCallState state)
{
    if (state == RKLLM_RUN_ERROR) {
        emit llmResultReady("[LLM推理错误]");
        m_fullText.clear();
        return;
    }

    if (state == RKLLM_RUN_WAITING || state == RKLLM_RUN_NORMAL) {
        if (result->text) {
            m_fullText += QString::fromUtf8(result->text);
        }

        // 可选保存隐藏层
        if (result->last_hidden_layer.embd_size != 0 && result->last_hidden_layer.num_tokens != 0) {
            int size = result->last_hidden_layer.embd_size * result->last_hidden_layer.num_tokens * sizeof(float);
            QFile file("last_hidden_layer.bin");
            if (file.open(QIODevice::WriteOnly)) {
                file.write((const char *)result->last_hidden_layer.hidden_states, size);
                file.close();
            }
        }

        return;
    }

    if (state == RKLLM_RUN_FINISH) {
        emit llmResultReady(m_fullText.trimmed());

        m_fullText.clear();
    }
}

