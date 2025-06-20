---
主窗口显示YOLO的结果，需要创建个Qt线程实现RTMP直播推流
---

# 一、需求

主窗口显示YOLO的结果，其成员函数 MainWindow::updateFrame() 用来将 YOLO 的结果显示在 video_label 中。

我们需要创建一个新的线程，实现 RTMP 推流。



# 二、实现

- 创建类 RTMPThread 继承 QThread
- 成员函数应有：
  - 开始推流 startStream()，当信号 isRunning = true 的时候
  - 结束推流 stopStream()， 当信号 isRunning = false 的时候
  - 获取当前帧 void setCurrentFrame(const cv::Mat& frame)，需要在 MainWindow::updateFrame() 函数中截胡帧数据。
- 重写 run()
- 主窗口
  - 初始化 RTMP 线程
  - 连接信号，可以用按键
  - 也可以在开启视频流的时候，开启推流

