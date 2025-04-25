---
基础教程1之Hello world
---



# 一、源码

创建文件：basic-tutorial-1.c

```c
#include <gst/gst.h>
 
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
 
int
tutorial_main (int argc, char *argv[])
{
  GstElement *pipeline;
  GstBus *bus;
  GstMessage *msg;
 
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
 
  /* Build the pipeline */
  pipeline =
      gst_parse_launch
      ("playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm",
      NULL);
 
  /* Start playing */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
 
  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg =
      gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
      GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
 
  /* See next tutorial for proper error message handling/parsing */
  if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
    g_printerr ("An error occurred! Re-run with the GST_DEBUG=*:WARN "
        "environment variable set for more details.\n");
  }
 
  /* Free resources */
  gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}
 
int
main (int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
  return gst_macos_main ((GstMainFunc) tutorial_main, argc, argv, NULL);
#else
  return tutorial_main (argc, argv);
#endif
}
```



**编译**

```
gcc basic-tutorial-1.c -o basic-tutorial-1 `pkg-config --cflags --libs gstreamer-1.0`
```

**执行**

```
LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0 ./basic-tutorial-1
```

# 二、源码分析

## 1. 初始化

```c
/* Initialize GStreamer */
  gst_init (&argc, &argv);
```

 这必须始终是您的第一个Gstreamer执行语句，它的功能如下：

- 初始化所有内部结构
- 检查可用的插件
- 执行任何用于GStreamer的命令行选项



## 2. 创建管道

```c
/* Build the pipeline */
  pipeline = gst_parse_launch ("playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm",NULL);
```



​		GStreamer是一个设计用来处理多媒体流的框架。媒体从“源”元素（生产者）到“汇”元素（消费者），经过一系列执行各种任务的中间元素。所有相互连接的元素的集合称为“管道”。

​		在GStreamer中，您通常通过手动组装各个元素来构建管道，但是，当管道足够简单并且您不需要任何高级特性时，您可以采用快捷方式：gst_parse_launch（）。这个函数接受管道的文本表示，并将其转换为实际的管道，这非常方便。

​		那么，我们要求gst_parse_launch（）为我们构建什么样的管道呢？这里进入第二个关键点：我们正在构建一个由称为playbin的单个元素组成的管道。

​		Playbin是一个特殊的元素，它既是源又是汇，是一个完整的管道。在内部，它创建并连接播放媒体所需的所有必要元素，因此您不必担心它。



## 3. 设置管道状态

```c
/* Start playing */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
```

在gst_element_set_state（）将`pipeline` （记住，我们唯一的元素）设置PLAYING状态，从而启动播放。



## 4. 获取管道总线信息

```c
/* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg =gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
```

​	这些行将等待，直到出现错误或找到流的结尾。gst_element_get_bus（）检索管道的总线，gst_bus_timed_pop_filtered（）将阻塞，直到您通过该总线接收到ERROR或EOS （end - stream）。不要太担心这一行，GStreamer总线在基础教程2:GStreamer概念中有解释。



## 5. 清理

```c
/* Free resources */
  gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
```

gst_bus_timed_pop_filtered（）返回了一条需要用gst_message_unref（）释放的消息.

gst_element_get_bus（）获取的总线，必须使用gst_object_unref（）释放。

最后，取消对管道的引用将破坏管道及其所有内容。

# 小结

- 如何使用 gst_init（），初始化GStreamer。
- 如何使用 gst_parse_launch（），从文本描述快速构建管道。
- 如何使用 playbin创建自动播放管道。
- 如何使用 gst_element_set_state（），向GStreamer发送信号以开始播放。
- 如何使用 gst_element_get_bus（）和gst_bus_timed_pop_filtered（），获取管道总线和获取管道总线信息。
- 如何使用 gst_message_unref（）和gst_object_unref（），清理GStreamer资源
  



