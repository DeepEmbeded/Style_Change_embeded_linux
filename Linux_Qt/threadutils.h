#ifndef THREADUTILS_H
#define THREADUTILS_H
#include <QThread>

void bindThreadToCore(QThread *thread, int coreId);
#endif // THREADUTILS_H
