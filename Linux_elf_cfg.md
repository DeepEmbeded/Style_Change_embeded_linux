---
Linux_elf 的开发环境搭建
---





# 一、解压SDK包

通过 FileZilla 传输 ELF2-linux-source.tar.bz2.*

合并分卷文件

```
cat ELF2-linux-source.tar.bz2.* > ELF2-linux-source.tar.bz2
```



解压合并后的文件

```
tar -xjvf ELF2-linux-source.tar.bz2
```



# 二、安装Gstream

## 1. 安装 GStream核心库

```bash
sudo apt update
sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
gstreamer1.0-libav gstreamer1.0-tools
```

- libgstreamer1.0-dev：开发头文件和库
- gstreamer1.0-tools：包含gst-launch-1.0等命令行工具
- 插件包：base（基础插件）、good（高质量插件）、bad（实验性插件）、ugly（专利编码插件）



## 2. 验证安装

```
gst-launch-1.0 --version  # 查看版本
gst-inspect-1.0 --plugin videoconvert  # 检查插件
```



## 3. 测试

**命令行测试**

```
gst-launch-1.0 playbin uri=file:///home/prover/GStream/video.mp4
```

**CPP调用API测试**

```c++
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GstElement *pipeline = gst_parse_launch(
        "playbin uri=file:///home/prover/GStream/video.mp4", 
        NULL
    );

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(
        bus, 
        GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)  // 关键修正
    );

    if (msg) {
        gst_message_unref(msg);
    }
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}

```

编译、运行：

```
g++ play.cpp -o play `pkg-config --cflags --libs gstreamer-1.0`
./play
```



