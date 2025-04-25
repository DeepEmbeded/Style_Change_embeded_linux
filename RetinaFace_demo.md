---
RetinaFace_demo 源码阅读
---

# 一、文件入口

## 1. 主函数

这部分砍掉关于图像的一切，换成下面的视频编解码。

### 1.1 参数检测

```c
if (argc != 3) {
        printf("%s <model_path> <image_path>\n", argv[0]);
        return -1;
    }

const char *model_path = argv[1];
const char *image_path = argv[2];
```

这部分改成视频的话：

```c
if (argc != 4)
  {
    printf("Usage: %s <model_path> <video_path> <video_type 264/265> \n", argv[0]);
    return -1;
  }
char *model_path = (char *)argv[1];
char *video_name = argv[2];
int video_type = atoi(argv[3]);
```

### 1.2 定义上下文结构体变量

```c
rknn_app_context_t rknn_app_ctx;
memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));

//结构体
typedef struct {
    rknn_context rknn_ctx;
    rknn_input_output_num io_num;
    rknn_tensor_attr *input_attrs;
    rknn_tensor_attr *output_attrs;
    int model_channel;
    int model_width;
    int model_height;
} rknn_app_context_t;
```

### 1.3 模型初始化

由于这边没有数据输入的参数，这边不需要改动。

```c
ret = init_retinaface_model(model_path, &rknn_app_ctx);
if (ret != 0) {
    printf("init_retinaface_model fail! ret=%d model_path=%s\n", ret, model_path);
    return -1;
}
```

### 1.4 配置 Mpp 解码器

这边既然要处理视频，那么就肯定要用到解码器。

```c
if (app_ctx.decoder == NULL)
{
    MppDecoder *decoder = new MppDecoder();
    decoder->Init(video_type, 30, &app_ctx);
    decoder->SetCallback(mpp_decoder_frame_callback);
    app_ctx.decoder = decoder;
}
```

### 1.5 初始化视频输出文件

```c
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
```

### 1.6 视频输入源

```c
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
```

> 这里留给后面用 GStreamer 调用 v4l2 获取摄像头数据

### 1.7 释放编解码资源

```c
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



## 2. Decoder 帧回调

模型推理和视频编码放在解码后处理这里。

### 2.1 Encoder 编码器初始化

```c
if (ctx->encoder == NULL) {
    MppEncoder *mpp_encoder = new MppEncoder();
    // 设置编码参数（H.264, YUV420SP格式）
    MppEncoderParams enc_params;
    memset(&enc_params, 0, sizeof(MppEncoderParams));
    enc_params.width = width;
    enc_params.height = height;
    enc_params.hor_stride = width_stride;
    enc_params.ver_stride = height_stride;
    enc_params.fmt = MPP_FMT_YUV420SP;
    enc_params.type = MPP_VIDEO_CodingAVC; // RK3562仅支持H.264
    mpp_encoder->Init(enc_params, NULL);
    ctx->encoder = mpp_encoder;
}
```

### 2.2 模型推理

原来的代码中，模型推理这块是放在 main.cc 的，现在放在帧回调这块。因为帧也是图片，就不需要改动什么。

```c
ret = inference_retinaface_model(&rknn_app_ctx, &src_image, &result);
if (ret != 0) {
    printf("init_retinaface_model fail! ret=%d\n", ret);
    goto out;
}
```



### 2.3 帧数据处理

```c
// 获取编码器输入缓冲区
mpp_frame = ctx->encoder->GetInputFrameBuffer();
mpp_frame_fd = ctx->encoder->GetInputFrameBufferFd(mpp_frame);
mpp_frame_addr = ctx->encoder->GetInputFrameBufferAddr(mpp_frame);

// 使用RGA（Rockchip Graphic Accelerator）复制帧数据
origin = wrapbuffer_fd(fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
src = wrapbuffer_fd(mpp_frame_fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
imcopy(origin, src);
```



### 2.4 绘制人脸框

原来的代码：

```c
for (int i = 0; i < result.count; ++i) {
    int rx = result.object[i].box.left;
    int ry = result.object[i].box.top;
    int rw = result.object[i].box.right - result.object[i].box.left;
    int rh = result.object[i].box.bottom - result.object[i].box.top;
    draw_rectangle(&src_image, rx, ry, rw, rh, COLOR_GREEN, 3);
    char score_text[20];
    snprintf(score_text, 20, "%0.2f", result.object[i].score);
    printf("face @(%d %d %d %d) score=%f\n", result.object[i].box.left, result.object[i].box.top,
           result.object[i].box.right, result.object[i].box.bottom, result.object[i].score);
    draw_text(&src_image, score_text, rx, ry, COLOR_RED, 20);
    for(int j = 0; j < 5; j++) {
    draw_circle(&src_image, result.object[i].ponit[j].x, result.object[i].ponit[j].y, 2, COLOR_ORANGE, 4);
    }
}
```

draw_rectangle、draw_text和 draw_circle 处理的数据指针 src_image 需要改成 (unsigned char *)mpp_frame_addr

## 2.5 编码输出

```c
// 第一帧写入编码头信息
if (frame_index == 1) {
    enc_data_size = ctx->encoder->GetHeader(enc_data, enc_buf_size);
    fwrite(enc_data, 1, enc_data_size, ctx->out_fp);
}

// 编码帧数据
memset(enc_data, 0, enc_buf_size);
enc_data_size = ctx->encoder->Encode(mpp_frame, enc_data, enc_buf_size);
fwrite(enc_data, 1, enc_data_size, ctx->out_fp);
```

