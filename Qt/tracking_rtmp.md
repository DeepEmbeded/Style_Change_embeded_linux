# 前言

开发环境：RK3588 buildroot 

第三方包：opencv集成gstreamer 

编码器：Rockchip mpp 

GUI：Qt 

摄像头：MIPI CSI 

---

# 一、视频处理

## 1. 流程

- **V4L2 采集**：通过 v4l2src 获取NV12格式原始数据
- **OpenCV处理**：使用 appsink 接收数据，保持NV12格式避免转换开销
- **MPP编码**：通过 mpph264enc 直接接收NV12格式
- **推流输出**：编码后的 H.264 流经 h264parse 和 flvmux 封装后推送RTMP

```
[MIPI 摄像头]
     ↓ V4L2 (video11)
[NV12: YUV420SP]
     ↓
[OpenCV (cvtColor)]
 → [BGR 图像用于显示/UI]
 → [RGB/BGR 图像送 YOLO]
     ↓
[QImage 显示: RGB/BGR888]

或（并行路径）

[NV12 原图]
     ↓
[mpph264enc 直接编码]
     ↓
[GStreamer 推流]

```



## 2. 推流管道

```cpp
    // 摄像头设备
    const std::string dev = "/dev/video11";

    // GStreamer推流管道，使用mpph264enc，注意尺寸和帧率根据实际调整
    std::string gst_pipeline =
        "appsrc ! "
        "videoconvert ! "
        "video/x-raw,format=I420 ! "    // mpph264enc 要求格式，opencv默认NV12，转I420格式
        "mpph264enc ! "
        "h264parse ! "
        "flvmux ! "
        "rtmpsink location=rtmp://192.168.10.50/live/stream";

    cv::VideoCapture cap(dev, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        qWarning("打开摄像头失败");
        emit finished();
        return;
    }

	// 设置摄像头分辨率（根据实际摄像头调整）
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('N','V','1','2')); // NV12

    // 打开VideoWriter推流
    cv::VideoWriter writer;
    bool opened = writer.open(gst_pipeline, 0, 30.0, cv::Size(1280, 720), true);
    if (!opened) {
        qWarning("无法打开GStreamer推流管道");
        cap.release();
        emit finished();
        return;
    }
```

这边摄像头不要占用 gstreamer 管道，直接用 cv 的 VideoCapture 即可。



# 二、Qt 显示主线程



# 三、触摸事件

```
YOLO推理（每帧） → 显示检测框（蓝色） → 
用户点击框 → 标记目标框为高亮（红框） → 
初始化 OpenCV Tracker → 
后续帧中用 Tracker 更新红框位置 → 
绘制追踪框（红）并不断更新
```



## 1. 新增结构体

**原来的结构体**

```cpp
typedef struct {
    image_rect_t box;
    float prop;
    int cls_id;
} object_detect_result;
```



**修改后**

需要添加个是否选中的 flag。

```cpp
typedef struct {
    image_rect_t box;
    float prop;
    int cls_id;
    bool selected;	//新增：是否被触摸选中
} object_detect_result;
```



## 2. 触摸检测

在YOLOv5类的成员函数中，增加一个 `handleTouch(x, y)` 函数。

```cpp
void YOLOv5Detector::handleTouch(int touchX, int touchY) {
    for (int i = 0; i < results->count; ++i) {
        object_detect_result *detResult = &(results->results[i]);
        int x1 = res.box.left;
        int y1 = res.box.top;
        int x2 = res.box.right;
        int y2 = res.box.bottom;
        QRect boxRect(x1, y1, x2 - x1, y2 - y1);

        if (boxRect.contains(touchX, touchY)) {
            res.selected = true;
            qDebug() << "Touch selected box at:" << boxRect;
        }
    }
}

```



## 3. 绘制函数

有被选中的框就变红色。

```
void YOLOv5Detector::processDetectionResults(image_buffer_t *srcImage, extended_result_list *results) {
    char text[256];
    for (int i = 0; i < results->count; i++) {
        extended_detect_result *res = &(results->results[i]);
        int x1 = res->box.left;
        int y1 = res->box.top;
        int x2 = res->box.right;
        int y2 = res->box.bottom;

        // 蓝色或红色
        color_t boxColor = res->selected ? COLOR_RED : COLOR_BLUE;

        draw_rectangle(srcImage, x1, y1, x2 - x1, y2 - y1, boxColor, 3);
        sprintf(text, "%s %.1f%%", coco_cls_to_name(res->id), res->prop * 100);
        draw_text(srcImage, text, x1, y1 - 20, COLOR_RED, 10);
    }
}

```



## 4. 定时器

主窗口添加：

```cpp
QTimer *printTimer = new QTimer(this);
connect(printTimer, &QTimer::timeout, this, [=]() {
    for (int i = 0; i < detected_results.count; ++i) {
        auto &res = detected_results.results[i];
        if (res.selected) {
            qDebug() << "Selected:" << coco_cls_to_name(res.id)
                     << "Box:" << res.box.left << res.box.top << res.box.right << res.box.bottom;
        }
    }
});
printTimer->start(500);

```



## 5. 触摸事件

对 inferLabel 进行重写 mousePressEvent()

```
void ImageLabel::mousePressEvent(QMouseEvent *event) {
    QPoint pos = event->pos();
    emit userTouched(pos.x(), pos.y());
}
```



```
connect(imageLabel, &ImageLabel::userTouched, detector, &YOLOv5Detector::handleTouch);
```



