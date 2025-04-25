---
2. Gstreamer 概念

---

# 一、元素创建

以下元素是 Gst 的基本构造快。他们处理 数据从源元素（数据生产者）通过 filter 元素流向下游 传递给 sink 元素（数据使用者）。

![img](https://gstreamer.freedesktop.org/documentation/tutorials/basic/images/figure-1.png)

```c
  /* Create the elements */
  source = gst_element_factory_make ("videotestsrc", "source");
  sink = gst_element_factory_make ("autovideosink", "sink");
```

上个demo是通过 gst_parse_launch() 来创建管道。后面我们基本都是要用 gst_element_factory_make()。

- 第一个参数是 元素的几种常见的类型，比如之后要用到的 v4l2src。
- 第二个参数是 name 来指定。

本节demo并没有filter，只创建 创建两个元素：[videotestsrc](https://gstreamer.freedesktop.org/documentation/videotestsrc/index.html#videotestsrc) 和 [一个 autovideosink](https://gstreamer.freedesktop.org/documentation/autodetect/autovideosink.html#autovideosink)

![img](https://gstreamer.freedesktop.org/documentation/tutorials/basic/images/basic-concepts-pipeline.png)

-  videotestsrc：是一个源元素（它生成数据），它会创建一个 测试视频模式。此元素可用于调试目的，在实际应用程序中通常找不到。
- autovideosink：是一个 sink 元素（它使用数据），它显示在 一个窗口接收的图像。存在多个视频接收器， 取决于操作系统，具有不同的功能范围。



# 二、管道创建

```c
  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
```

Gst 中的所有元素在使用前，必须包含在管道内。管道会处理一些计时和消息传递，使用 gst_pipeline_new() 创建管道。

```c
  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, sink, NULL);
  if (gst_element_link (source, sink) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
```

管道是一种特定类型的 bin，用于包含其他元素。

本demo中，调用 gst_bin_add_many() 添加元素到管道中。该函数接收一个列表的元素，以NULL结尾。当然也可以用 gst_bin_add() 单独添加。

添加到管道后，还需要将元素之间相互链接。使用 gst_element_link()，source 和 sink 元素的顺序不能反。



# 三、元素属性

大多数 Gst 元素都具有可自定义的属性：**命名属性** 可以修改元素的行为。

```c
  /* Modify the source's properties */
  g_object_set (source, "pattern", 0, NULL);
```

上面使用 g_object_set 更改了 [videotestsrc](https://gstreamer.freedesktop.org/documentation/videotestsrc/index.html#videotestsrc) 的 “pattern” 属性， 它控制元素输出的测试视频的类型。



# 四、错误检测

上一节用 gst_element_set_state() 是没有检测其返回值的。

```c
/* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }
```

gst_bus_timed_pop_filtered() 等待执行结束并返回 GstMessage，在 GStreamer 遇到错误条件或 EOS，因此我们需要检查 发生了哪一个，并在屏幕上打印一条消息。

GstMessage 可以虚拟交付任何类型的消息，且GStreamer 提供了一系列 解析每种消息的函数。

```c
  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg =
      gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
      GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n",
            GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n",
            debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        /* We should not reach here because we only asked for ERRORs and EOS */
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }
```



# 五、总线

 GStreamer 总线 负责将 元素生成的 GstMessage 按顺序和应用程序线程交付给应用程序对象。

消息可以与 gst_bus_timed_pop_filtered 及其同级同步从总线中提取，也可以使用信号异步提取。



# 小结

本教程展示了：

- 如何使用 [gst_element_factory_make](https://gstreamer.freedesktop.org/documentation/gstreamer/gstelementfactory.html#gst_element_factory_make) 创建元素()
- 如何使用 [gst_pipeline_new](https://gstreamer.freedesktop.org/documentation/gstreamer/gstpipeline.html#gst_pipeline_new) 创建空管道()
- 如何使用 [gst_bin_add_many](https://gstreamer.freedesktop.org/documentation/gstreamer/gstbin.html#gst_bin_add_many) 将元素添加到管道中()
- 如何使用 [gst_element_link](https://gstreamer.freedesktop.org/documentation/gstreamer/gstelement.html#gst_element_link) 将元素彼此链接起来()