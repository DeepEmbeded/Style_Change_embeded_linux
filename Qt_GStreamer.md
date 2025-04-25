---
Qt 集成 Gstreamer

---

# 一、配置 Gstreamer 工程

在 .Pro 文件中添加如下：

```qmake
# 包含路径
INCLUDEPATH += /usr/include/glib-2.0 \
               /usr/lib/x86_64-linux-gnu/glib-2.0/include \
               /usr/include/gstreamer-1.0 \
               /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include

# 链接库
LIBS += -lgstreamer-1.0 -lgstvideo-1.0 -lglib-2.0 -lgobject-2.0
LIBS += -L/usr/lib/x86_64-linux-gnu/gstreamer-1.0 -lgstautodetect
```



# 二、通过 v4l2 获取Camera视频

## 1. 基本实现

```c
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *capsfilter, *sink;
    GstCaps *caps;
    GstBus *bus;
    GstMessage *msg;

    /* 初始化GStreamer */
    gst_init(&argc, &argv);

    /* 创建元素 */
    pipeline = gst_pipeline_new("video-pipeline");
    source = gst_element_factory_make("v4l2src", "video-source");
    capsfilter = gst_element_factory_make("capsfilter", "filter");
    sink = gst_element_factory_make("autovideosink", "video-output");

    /* 检查元素创建是否成功 */
    if (!pipeline || !source || !capsfilter || !sink) {
        g_printerr("无法创建元素\n");
        return -1;
    }

    /* 设置v4l2src设备参数 */
    g_object_set(G_OBJECT(source), "device", "/dev/video11", NULL);

    /* 创建并设置capsfilter参数 */
    caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "NV12",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL);
    g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
    gst_caps_unref(caps);

    /* 构建管道 */
    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, sink, NULL);
    if (!gst_element_link_many(source, capsfilter, sink, NULL)) {
        g_printerr("无法链接元素\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* 启动管道 */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* 等待错误或EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    /* 释放资源 */
    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}

```



## 2. 集成到 Qt 中

```cpp
#include <QApplication>
#include <QVideoWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

class VideoPlayer : public QWidget {
    Q_OBJECT
public:
    VideoPlayer(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        setupGStreamer();
    }

    ~VideoPlayer() {
        if (pipeline) {
            gst_element_set_state(pipeline, GST_STATE_NULL);
            gst_object_unref(pipeline);
        }
    }

private slots:
    void togglePlayback() {
        if (isPlaying) {
            gst_element_set_state(pipeline, GST_STATE_PAUSED);
            playButton->setText("开始");
        } else {
            gst_element_set_state(pipeline, GST_STATE_PLAYING);
            playButton->setText("暂停");
        }
        isPlaying = !isPlaying;
    }

private:
    void setupUI() {
        QVBoxLayout *layout = new QVBoxLayout(this);
        
        videoWidget = new QVideoWidget();
        videoWidget->setMinimumSize(640, 480);
        
        playButton = new QPushButton("开始");
        connect(playButton, &QPushButton::clicked, this, &VideoPlayer::togglePlayback);
        
        layout->addWidget(videoWidget);
        layout->addWidget(playButton);
        
        setLayout(layout);
        resize(800, 600);
    }

    void setupGStreamer() {
        gst_init(nullptr, nullptr);
        
        /* 创建管道元素 */
        pipeline = gst_pipeline_new("video-pipeline");
        GstElement *source = gst_element_factory_make("v4l2src", "source");
        GstElement *capsfilter = gst_element_factory_make("capsfilter", "filter");
        GstElement *sink = gst_element_factory_make("xvimagesink", "sink");
        
        if (!pipeline || !source || !capsfilter || !sink) {
            qCritical("无法创建GStreamer元素");
            return;
        }

        /* 配置元素参数 */
        g_object_set(source, "device", "/dev/video0", NULL);
        
        GstCaps *caps = gst_caps_new_simple("video/x-raw",
            "width", G_TYPE_INT, 640,
            "height", G_TYPE_INT, 480,
            "framerate", GST_TYPE_FRACTION, 30, 1,
            NULL);
        g_object_set(capsfilter, "caps", caps, NULL);
        gst_caps_unref(caps);

        /* 构建管道 */
        gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, sink, NULL);
        if (!gst_element_link_many(source, capsfilter, sink, NULL)) {
            qCritical("无法链接元素");
            gst_object_unref(pipeline);
            pipeline = nullptr;
            return;
        }

        /* 绑定到Qt窗口 */
        WId winId = videoWidget->winId();
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), winId);
        
        isPlaying = false;
    }

    QVideoWidget *videoWidget;
    QPushButton *playButton;
    GstElement *pipeline = nullptr;
    bool isPlaying;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    VideoPlayer player;
    player.show();
    
    return app.exec();
}

#include "main.moc"  // 用于Qt的元对象系统
```



# 三、视频流进行 yolo 推理

yolo源码添加：

```c
int process_video_buffer(rknn_app_context_t *ctx, uint8_t *data, int size) {
    const int CHUNK_SIZE = 8192;
    uint8_t *data_ptr = data;
    uint8_t *data_end = data + size;

    do {
        int pkt_eos = 0;
        int chunk_size = CHUNK_SIZE;
        
        if (data_ptr + chunk_size >= data_end) {
            pkt_eos = 1;
            chunk_size = data_end - data_ptr;
        }

        ctx->decoder->Decode(data_ptr, chunk_size, pkt_eos);
        data_ptr += chunk_size;

        if (data_ptr >= data_end) {
            printf("reset decoder\n");
            break;
        }
    } while (1);

    return 0;
}
```

v4l2 捕获后调用该函数

```c
// 在V4L2捕获循环中直接调用
uint8_t *frame_data = (uint8_t*)mmap(...);  // V4L2内存映射
int frame_size = buf.length;
process_video_buffer(ctx, frame_data, frame_size);
```

对视频流进行 yolo 推理，最好封装个类：class yolo_process;

```c
rknn_app_context_t app_ctx;
memset(&app_ctx, 0, sizeof(rknn_app_context_t));

ret = init_model(model_name, &app_ctx);
if (ret != 0)
{
    printf("init model fail\n");
    return -1;
}
  if (app_ctx.decoder == NULL)
  {
    MppDecoder *decoder = new MppDecoder();
    decoder->Init(video_type, 30, &app_ctx);
    decoder->SetCallback(mpp_decoder_frame_callback);
    app_ctx.decoder = decoder;
  }

  if (app_ctx.out_fp == NULL)
  {
    FILE *fp = fopen(OUT_VIDEO_PATH, "w");
    if (fp == NULL)
    {
      printf("open %s error\n", OUT_VIDEO_PATH);
      return -1;
    }
    app_ctx.out_fp = fp;
  }

  printf("app_ctx=%p decoder=%p\n", &app_ctx, app_ctx.decoder);

  if (strncmp(video_name, "rtsp", 4) == 0)
  {
#if defined(BUILD_VIDEO_RTSP)
    process_video_rtsp(&app_ctx, video_name);
#else
    printf("rtsp no support\n");
#endif
  }
  else
  {
    process_video_file(&app_ctx, video_name);
  }
   printf("waiting finish\n");
  usleep(3 * 1000 * 1000);

  // release
  fflush(app_ctx.out_fp);
  fclose(app_ctx.out_fp);

  if (app_ctx.decoder != nullptr)
  {
    delete (app_ctx.decoder);
    app_ctx.decoder = nullptr;
  }
  if (app_ctx.encoder != nullptr)
  {
    delete (app_ctx.encoder);
    app_ctx.encoder = nullptr;
  }

  release_model(&app_ctx);
```



# 四、将视频推流服务器

使用Gstreamer使用协议rtmp推流。

```c
// 初始化GStreamer管道（示例）
GstElement *pipeline = gst_parse_launch(
    "appsrc name=source ! h264parse ! flvmux ! rtmpsink location=rtmp://localhost/live/stream",
    NULL
);
GstElement *appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "source");

// 在编码后推送数据
GstBuffer *buffer = gst_buffer_new_wrapped(enc_data, enc_data_size);
gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);
```



# 五、坐标数据传输

通过 Uart 进行对 STM32 的数据传输。

得到数据：

```c
int center_x = detection.x + (detection.width / 2);
int center_y = detection.y + (detection.height / 2);
```

RK3588端：

```c
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

int uart_send(const char *dev_path, int center_x, int center_y) {
    int fd = open(dev_path, O_WRONLY | O_NOCTTY);
    if (fd < 0) {
        perror("Failed to open UART device");
        return -1;
    }

    // 配置串口
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);  // 波特率 115200
    cfsetospeed(&options, B115200);
    options.c_cflag &= ~PARENB;      // 无校验位
    options.c_cflag &= ~CSTOPB;      // 1 位停止位
    options.c_cflag &= ~CSIZE;       // 8 位数据位
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);

    // 发送数据（格式： "X,Y\n"）
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d,%d\n", center_x, center_y);
    write(fd, buffer, strlen(buffer));

    close(fd);
    return 0;
}
```

STM32 端

```c
#include "stm32f4xx_hal.h"

UART_HandleTypeDef huart1;  // 假设使用 USART1
uint8_t rx_buffer[32];      // 接收缓冲区

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart1) {
        // 解析坐标（格式： "X,Y\n"）
        int center_x, center_y;
        sscanf((char*)rx_buffer, "%d,%d", &center_x, &center_y);

        // 处理坐标（例如控制电机或舵机）
        process_coordinates(center_x, center_y);

        // 重新启动接收
        HAL_UART_Receive_IT(&huart1, rx_buffer, sizeof(rx_buffer));
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();  // 初始化 UART

    // 启动 UART 接收（中断模式）
    HAL_UART_Receive_IT(&huart1, rx_buffer, sizeof(rx_buffer));

    while (1) {
        // 主循环
    }
}
```

