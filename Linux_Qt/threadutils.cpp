#include "threadutils.h"
#include <pthread.h>
#include <sched.h>
#include <QDebug>

void bindThreadToCore(QThread *thread, int coreId)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);

    pthread_t handle = (pthread_t)thread->currentThreadId();

    int rc = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        qDebug() << "[CoreBinding] Failed to bind thread to CPU" << coreId << ", error:" << rc;
    } else {
        qDebug() << "[CoreBinding] Thread bound to CPU" << coreId;
    }
}
