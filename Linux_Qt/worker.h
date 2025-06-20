#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <QThread>

class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker();

public slots:
    void process();  // 线程主函数
    void stop();     // 停止线程

signals:
    void frameReady(const QImage &img);
    void finished();

    //void matReady(const QImage &img);  // 新增：发送cv::Mat给推理模块


private:
    bool running;
};

#endif // WORKER_H
