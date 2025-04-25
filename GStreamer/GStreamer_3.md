---
3. 动态管道

---

# 一、介绍

本demo，我们将打开一个多路复用的文件。意思是，音频和视频一起存储在一个容器文件中。负责打开此容器的元素称为解复用器。

如果容器嵌入了多个流（例如：一个视频轨道和两个音频轨道），解复用器将他们分开并通过不同的输出端口。如此，可以创建不同的分支在管道中，处理不同类型的数据。

元素之间互相通信的端口 叫 pads，source elements 只包含 source pads、sink 元素仅包含 sink pad，filter elements 包含 两者。

![img](https://gstreamer.freedesktop.org/documentation/tutorials/basic/images/simple-player.png)



# 二、demo分析

## 1. 基本设置

用一个结构体包含我们的所有信息。

```c
/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GstElement *pipeline;
  GstElement *source;
  GstElement *convert;
  GstElement *resample;
  GstElement *sink;
} CustomData;
```

前向引用。

```c
/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);
```



解复用器和解码器 uridecodebin，playbin。

audioconvert ：对于在不同音频格式之间进行转换非常有用，确保此示例适用于任何平台，因为音频解码器生成的格式可能与音频接收器预期的格式不同。

audioResample：可用于在不同音频采样率之间进行转换，同样确保此示例适用于任何平台，因为音频解码器生成的音频采样率可能不是音频接收器支持的速率。

autoaudiosink 和上个 demo 的 autovideosink类似，它会将音频流呈现到音频卡。

```c
/* Create the elements */
data.source = gst_element_factory_make ("uridecodebin", "source");
data.convert = gst_element_factory_make ("audioconvert", "convert");
data.resample = gst_element_factory_make ("audioresample", "resample");
data.sink = gst_element_factory_make ("autoaudiosink", "sink");
```

在这块，我们链接了 elements converter、resample 和 sink，但我们**没有**将它们与 source 链接，因为此时它不包含 source pads。我们只是让这个分支 （converter + sink） 没有链接，直到以后。

```c
if (!gst_element_link_many (data.convert, data.resample, data.sink, NULL)) {
  g_printerr ("Elements could not be linked.\n");
  gst_object_unref (data.pipeline);
  return -1;
}
```

我们通过属性将文件的 URI 设置为播放

```c
/* Set the URI to play */
g_object_set (data.source, "uri", "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);
```



## 2. 信号

GSignals 是 GStreamer 中的一个关键点。它们允许您在发生有趣的事情时收到通知 （通过回调）。信号由名称标识。

```
/* Connect to the pad-added signal */
g_signal_connect (data.source, "pad-added", G_CALLBACK (pad_added_handler), &data);
```

我们附加到源（一个元素）的 “pad-added” 信号。为此，我们使用并提供要使用的回调函数 （） 和数据指针。GStreamer 对这个数据指针不做任何事情，它只是将其转发到回调，以便我们可以与它共享信息。在这种情况下，我们传递一个指向我们专门为此目的构建的结构的指针。

我们现在已经准备好了！只需将 pipeline 设置为 state，然后开始监听总线以获取有趣的消息（如 or），就像前面的教程一样。



## 3. 回调

当我们的 source 元素最终有足够的信息开始生成数据时，它将创建 source pads，并触发 “pad-added” 信号。此时，我们的回调将被调用：

```
static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) 
```

- `src` 是触发信号的 
- `new_pad` 是刚刚添加到元素中的。这通常是我们要链接到的 pad。
- `data` 是我们在附加到 Signal 时提供的指针。在此示例中，我们使用它来传递指针。



```
GstPad *sink_pad = gst_element_get_static_pad (data->convert, "sink");
```

我们从中提取 converter 元素，然后使用 检索其 sink pad 。现在我们将直接链接 pads。

```c
* If our converter is already linked, we have nothing to do here */
if (gst_pad_is_linked (sink_pad)) {
  g_print ("We are already linked. Ignoring.\n");
  goto exit;
}
```

现在我们将检查这个新 pad 将要输出的数据类型

```c
/* Check the new pad's type */
new_pad_caps = gst_pad_get_current_caps (new_pad, NULL);
new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
new_pad_type = gst_structure_get_name (new_pad_struct);
if (!g_str_has_prefix (new_pad_type, "audio/x-raw")) {
  g_print ("It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
  goto exit;
}
```

将新的 pad 链接到 sink_pad，当然两者需要被同一 bin 中的元素所拥有：

```c
/* Attempt the link */
ret = gst_pad_link (new_pad, sink_pad);
if (GST_PAD_LINK_FAILED (ret)) {
  g_print ("Type is '%s' but link failed.\n", new_pad_type);
} else {
  g_print ("Link succeeded (type '%s').\n", new_pad_type);
}
```

## 4. 状态

| 状态    | 描述                                                         |
| ------- | ------------------------------------------------------------ |
| NULL    | 元素的 NULL 状态或起始状态                                   |
| READY   | 元素已准备好转到 PAUSED                                      |
| PAUSED  | 元素已暂停，则准备好接收和处理数据。sink元素只接受一个 buffer。 |
| PLAYING | 元素正在PLAYING，时钟正在运行，数据正在流动。                |



```c
case GST_MESSAGE_STATE_CHANGED:
  /* We are only interested in state-changed messages from the pipeline */
  if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
    g_print ("Pipeline state changed from %s to %s:\n",
        gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
  }
  break；
```

该代码用来理解几个状态的转换。



## 小结

在本节中，您学习了：

- 如何使用`GSignals`
- 如何直接连接 sink 。`GstPad`
- GStreamer 元素的各种状态。





