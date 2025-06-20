#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H
// safequeue.h
#pragma once
#include <QMutex>
#include <QWaitCondition>
#include <QVector>
#include <queue>
#include <QQueue>

// SafeQueue示例（可用你之前实现的）
template<typename T>
class SafeQueue {
public:
    void push(const T& item) {
        QMutexLocker locker(&m_mutex);
        m_queue.enqueue(item);
        m_waitCondition.wakeOne();
    }
    bool waitAndPop(T& out, int timeoutMs = 1000) {
        QMutexLocker locker(&m_mutex);
        if (m_queue.isEmpty()) {
            if (!m_waitCondition.wait(&m_mutex, timeoutMs))
                return false;
        }
        if (m_queue.isEmpty())
            return false;
        out = m_queue.dequeue();
        return true;
    }
    void wakeAll() {
        QMutexLocker locker(&m_mutex);
        m_waitCondition.wakeAll();
    }
private:
    QQueue<T> m_queue;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
};

#endif // SAFEQUEUE_H
