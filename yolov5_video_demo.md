---

学习Yolov5的demo
---

# 一、RKMpp 框架

RKMpp 大致跟隔壁 海思的HiMpp框架相差不会太多。

![img](https://i-blog.csdnimg.cn/direct/c3117c406ec848ffb3c79f726069c50c.png)

其中的VI、VENC、VDEC、VO，RGA视频处理其实在海思那边接触过了，区别在于VPSS。



 **系统架构**

![img](https://i-blog.csdnimg.cn/direct/9220eef1a4a64ffbb503e7e07f2001a3.png)

- 硬件层Hardware
  - 硬件层是瑞芯微系列芯片平台的视频编解码硬件加速模块
- 内核驱动层Kernel driver
  - Linux内核的编码器硬件设备驱动，以及相关的mmu，内存，时钟，电源管理模块等。
- MPP层
  - 用户态的MPP层屏蔽了不同操作系统和不同芯片平台的差异，为上层使用者提供统一的MPI接口。
  - MPP层包括MPI模块、OSAL模块、HAL模块以及视频编解码器（Video Decoder / Video Encoder）和视频处理功能模块（Video Process）。
  - 这里的 MPI 其实就是指的是 MPP的 API 接口。
- 应用层
  - MPP层通过MPI对接各种中间件软件，如OpenMax、ffmpeg和gstreamer。当然后面选择使用 gstreamer，毕竟要做直播设备的。

# 二、Yolov5 video_demo 

## 1. 主函数

rknn_yolov5_demo/src 文件下：

- main.cc ：处理图像的
- main_video.cc：处理视频的

我们直接看 main_video.cc 的主函数部分：

### 1.1 **参数检测**

```c++
if (argc != 4)
  {
    printf("Usage: %s <rknn_model> <video_path> <video_type 264/265> \n", argv[0]);
    return -1;
  }
char *model_name = (char *)argv[1];
char *video_name = argv[2];
int video_type = atoi(argv[3]);
```

从这里也能看出，需要的3个参数分别是：RKNN转换的模型，视频路径，视频类型。



**结构体**

声明模型结构体，分配空间。

```c++
rknn_app_context_t app_ctx;
memset(&app_ctx, 0, sizeof(rknn_app_context_t));
```

结构体类型：

```c++
typedef struct
{
  rknn_context rknn_ctx;
  rknn_input_output_num io_num;
  rknn_tensor_attr *input_attrs;
  rknn_tensor_attr *output_attrs;
  int model_channel;
  int model_width;
  int model_height;
  FILE *out_fp;
  MppDecoder *decoder;
  MppEncoder *encoder;
} rknn_app_context_t;
```

- RKNN相关成员:
  - rknn_context rknn_ctx: RKNN模型上下文句柄
  - rknn_input_output_num io_num: 存储模型的输入输出数量信息
  - rknn_tensor_attr *input_attrs: 指向模型输入张量属性数组的指针
  - rknn_tensor_attr *output_attrs: 指向模型输出张量属性数组的指针
- 模型信息:
  - int model_channel: 模型期望的输入通道数(如3表示RGB)
  - int model_width: 模型期望的输入宽度
  - int model_height: 模型期望的输入高度
- 媒体处理相关:
  - MppDecoder *decoder: 视频解码器指针(基于Rockchip MPP框架)
  - MppEncoder *encoder: 视频编码器指针(基于Rockchip MPP框架)
  - FILE *out_fp: 输出文件指针，用于保存处理结果



### 1.2 **初始化模型**

```c++
ret = init_model(model_name, &app_ctx);
if (ret != 0)
{
    printf("init model fail\n");
    return -1;
}
```

查看第2小节初始化模型。



### 1.3 **配置Mpp解码器**

```cpp
if (app_ctx.decoder == NULL)
{
    MppDecoder *decoder = new MppDecoder();
    decoder->Init(video_type, 30, &app_ctx);
    decoder->SetCallback(mpp_decoder_frame_callback);
    app_ctx.decoder = decoder;
}
```

==查看第3小节查看 MppDecoder。而且模型推理也在这块，这块很重要。==

这边查看函数 `void mpp_decoder_frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data)`

这是一个MPP解码器的帧回调函数，当解码器完成一帧解码后会调用此函数。主要功能是处理解码后的视频帧并初始化编码器。

- 参数
  - userdata: 用户自定义数据指针，实际为rknn_app_context_t结构体
  - width_stride: 解码帧的水平步长（stride）
  - height_stride: 解码帧的垂直步长（stride）
  - width: 图像实际宽度
  - height: 图像实际高度
  - format: 图像格式
  - fd: 图像文件描述符
  - data: 图像数据指针



### 1.4 **初始化视频输出文件**

```cpp
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



### 1.5 **视频输入源**

```cpp
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

- RTSP流处理
- 本地文件处理

当然我们不对RTSP的拉流做任何处理，只做RTSP的推流。

函数 process_video_file，用到了 Decoder 的视频帧解码处理，可以查看 3.2节。

### 1.6 **释放资源**

```cpp
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

## 2. 初始化模型

### 2.1 **获取RKNN模型数据大小**

```c++
/* Create the neural network */
printf("Loading mode...\n");
int model_data_size = 0;
unsigned char *model_data = read_file_data(model_path, &model_data_size);
if (model_data == NULL)
{
return -1;
}
```

函数 `read_file_data `是个 char* 类型：

- 参数
  - filename: 要读取的文件路径
  - model_size: 输出参数，用于返回文件的大小（字节数）
- 返回值
  - 成功时返回指向文件数据的指针（unsigned char*）
  - 失败时返回NULL

### 2.2 **初始化 RKNN 上下文并加载模型**

```c++
ret = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
if (ret < 0)
{
printf("rknn_init error ret=%d\n", ret);
return -1;
}
```

函数 `int rknn_init(rknn_context* context, void* model, uint32_t size, uint32_t flag, rknn_init_extend* extend);`

- 参数
  - context: 输出参数，返回初始化后的 RKNN 上下文句柄
  - model: 输入参数，指向 RKNN 模型数据的指针
  - size: 输入参数，RKNN 模型数据的大小（字节数）
  - flag: 输入参数，初始化标志位，用于控制初始化行为
  - extend: 输入参数，扩展初始化参数（可选，可为 NULL）
- 返回值
  - 成功时返回 0 或 RKNN_SUCC
  - 失败时返回错误代码

### 2.3 **查询RKNN模型的各类信息**

```c++
//query the sdk & driver version
ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));

//query the number of input & output tensor
ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &app_ctx->io_num, sizeof(rknn_input_output_num));

//query the attribute of input tensor
ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));

//query the attribute of output tensor
ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
```

函数`int rknn_query(rknn_context context, rknn_query_cmd cmd, void* info, uint32_t size);`

- 参数
  - context: RKNN 上下文句柄
  - cmd: 查询命令，指定要查询的信息类型
  - info: 输出参数，存储查询结果的结构体指针
  - size: 输入参数，info 结构体的大小
- 返回
  - 成功返回 0 (RKNN_SUCC)
  - 失败返回错误代码

查看 cmd 的枚举类型：

```c++
typedef enum _rknn_query_cmd {
    RKNN_QUERY_IN_OUT_NUM = 0,                              /* query the number of input & output tensor. */
    RKNN_QUERY_INPUT_ATTR = 1,                              /* query the attribute of input tensor. */
    RKNN_QUERY_OUTPUT_ATTR = 2,                             /* query the attribute of output tensor. */
    RKNN_QUERY_PERF_DETAIL = 3,                             /* query the detail performance, need set
                                                               RKNN_FLAG_COLLECT_PERF_MASK when call rknn_init,
                                                               this query needs to be valid after rknn_outputs_get. */
    RKNN_QUERY_PERF_RUN = 4,                                /* query the time of run,
                                                               this query needs to be valid after rknn_outputs_get. */
    RKNN_QUERY_SDK_VERSION = 5,                             /* query the sdk & driver version */

    RKNN_QUERY_MEM_SIZE = 6,                                /* query the weight & internal memory size */
    RKNN_QUERY_CUSTOM_STRING = 7,                           /* query the custom string */

    RKNN_QUERY_NATIVE_INPUT_ATTR = 8,                       /* query the attribute of native input tensor. */
    RKNN_QUERY_NATIVE_OUTPUT_ATTR = 9,                      /* query the attribute of native output tensor. */

    RKNN_QUERY_NATIVE_NC1HWC2_INPUT_ATTR = 8,               /* query the attribute of native input tensor. */
    RKNN_QUERY_NATIVE_NC1HWC2_OUTPUT_ATTR = 9,              /* query the attribute of native output tensor. */

    RKNN_QUERY_NATIVE_NHWC_INPUT_ATTR = 10,                 /* query the attribute of native input tensor. */
    RKNN_QUERY_NATIVE_NHWC_OUTPUT_ATTR = 11,                /* query the attribute of native output tensor. */

    RKNN_QUERY_DEVICE_MEM_INFO = 12,                        /* query the attribute of rknn memory information. */

    RKNN_QUERY_INPUT_DYNAMIC_RANGE = 13,                    /* query the dynamic shape range of rknn input tensor. */
    RKNN_QUERY_CURRENT_INPUT_ATTR = 14,                     /* query the current shape of rknn input tensor, only valid for dynamic rknn model*/
    RKNN_QUERY_CURRENT_OUTPUT_ATTR = 15,                    /* query the current shape of rknn output tensor, only valid for dynamic rknn model*/

    RKNN_QUERY_CMD_MAX
} rknn_query_cmd;
```

### 2.4 **数据排列格式**

```c++
if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
  {
    printf("model is NCHW input fmt\n");
    app_ctx->model_channel = input_attrs[0].dims[1];
    app_ctx->model_height = input_attrs[0].dims[2];
    app_ctx->model_width = input_attrs[0].dims[3];
  }
  else
  {
    printf("model is NHWC input fmt\n");
    app_ctx->model_height = input_attrs[0].dims[1];
    app_ctx->model_width = input_attrs[0].dims[2];
    app_ctx->model_channel = input_attrs[0].dims[3];
  }
```

> NCHW 的含义：
>
> - N (Number): 批次中的样本数量
> - C (Channels): 通道数（如RGB图像的3个通道）
> - H (Height): 图像高度（像素行数）
> - W (Width): 图像宽度（像素列数）



## 3. MppDecoder

### 3.1 初始化 Decoder

函数 `int MppDecoder::Init(int video_type, int fps, void* userdata)`

用于初始化H.264/H.265视频解码器。

- 参数
  - video_type: 视频编码类型 (264表示H.264/AVC，265表示H.265/HEVC)
  - fps: 视频帧率
  - userdata: 用户自定义数据指针
- 返回值
  - 成功返回1
  - 失败返回-1或0（不同错误情况）



### 3.2 视频帧解码操作

```c
int MppDecoder::Decode(uint8_t* pkt_data, int pkt_size, int pkt_eos)
/**
 * @brief 执行视频解码操作
 * @param pkt_data 输入编码数据包指针
 * @param pkt_size 输入数据包大小
 * @param pkt_eos 是否结束流标记(End Of Stream)
 * @return 成功返回MPP_OK，失败返回错误码
 */
```



### 3.3 Decoder 帧回调

函数 `void mpp_decoder_frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data)`

- 参数
  - @brief MPP解码器帧回调函数，处理解码后的视频帧
   * @param userdata 用户数据指针，此处为rknn_app_context_t上下文
   * @param width_stride 帧的宽度步长（可能包含padding）
   * @param height_stride 帧的高度步长（可能包含padding）
   * @param width 帧的实际宽度
   * @param height 帧的实际高度
   * @param format 帧的格式（如YUV420等）
   * @param fd 帧缓冲区的文件描述符（DMA-BUF）
   * @param data 帧缓冲区的虚拟地址


#### 3.3.1 **编码器初始化**

```cpp
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

编码可以查看第 4 小结。

#### 3.3.2 **模型推理**

```cpp
image_frame_t img; // 准备图像帧结构
// ...填充图像参数...

detect_result_group_t detect_result;
memset(&detect_result, 0, sizeof(detect_result_group_t));

ret = inference_model(ctx, &img, &detect_result); // 执行目标检测
```

函数`static int inference_model(rknn_app_context_t *app_ctx, image_frame_t *img, detect_result_group_t *detect_result)`

- 参数
  - app_ctx: RKNN应用上下文，包含模型信息和运行时状态
  - img: 输入图像帧
  - detect_result: 用于存储检测结果的结构体

- 返回值
  - 返回值：成功返回0，失败返回-1


这部分具体看==第三节==。

#### 3.3.3 帧数据处理与绘制

```cpp
// 获取编码器输入缓冲区
mpp_frame = ctx->encoder->GetInputFrameBuffer();
mpp_frame_fd = ctx->encoder->GetInputFrameBufferFd(mpp_frame);
mpp_frame_addr = ctx->encoder->GetInputFrameBufferAddr(mpp_frame);

// 使用RGA（Rockchip Graphic Accelerator）复制帧数据
origin = wrapbuffer_fd(fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
src = wrapbuffer_fd(mpp_frame_fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
imcopy(origin, src);

// 绘制检测框
for (int i = 0; i < detect_result.count; i++) {
    detect_result_t *det_result = &(detect_result.results[i]);
    // 绘制红色矩形框
    draw_rectangle_yuv420sp((unsigned char *)mpp_frame_addr, width_stride, height_stride, 
                           det_result->box.left, det_result->box.top, 
                           det_result->box.right - det_result->box.left + 1, 
                           det_result->box.bottom - det_result->box.top + 1, 
                           0x00FF0000, 4);
}
```

绘制检测框，可以查询==第四节==。



#### 3.3.4 编码输出

```cpp
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



## 4. MppEncoder

### 4.1 初始化 Encoder

函数 `int MppEncoder::Init(MppEncoderParams& params, void* userdata) `

- 参数
  - MppEncoderParams& params: 编码器参数结构体引用
  - void* userdata: 用户自定义数据指针

**编码器结构体**：

```cpp
typedef struct
{
    RK_U32 width;
    RK_U32 height;
    RK_U32 hor_stride;
    RK_U32 ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;

    RK_U32 osd_enable;
    RK_U32 osd_mode;
    RK_U32 split_mode;
    RK_U32 split_arg;
    RK_U32 split_out;

    RK_U32 user_data_enable;
    RK_U32 roi_enable;

    // rate control runtime parameter
    RK_S32 fps_in_flex;
    RK_S32 fps_in_den;
    RK_S32 fps_in_num;
    RK_S32 fps_out_flex;
    RK_S32 fps_out_den;
    RK_S32 fps_out_num;
    RK_S32 bps;
    RK_S32 bps_max;
    RK_S32 bps_min;
    RK_S32 rc_mode;
    RK_S32 gop_mode;
    RK_S32 gop_len;
    RK_S32 vi_len;

    /* general qp control */
    RK_S32 qp_init;
    RK_S32 qp_max;
    RK_S32 qp_max_i;
    RK_S32 qp_min;
    RK_S32 qp_min_i;
    RK_S32 qp_max_step; /* delta qp between each two P frame */
    RK_S32 qp_delta_ip; /* delta qp between I and P */
    RK_S32 qp_delta_vi; /* delta qp between vi and P */

    RK_U32 constraint_set;
    RK_U32 rotation;
    RK_U32 mirroring;
    RK_U32 flip;

    MppEncHeaderMode header_mode;
    MppEncSeiMode sei_mode;
} MppEncoderParams;
```

### 4.2 获取视频编码器头部信息

```c
int MppEncoder::GetHeader(char* enc_buf, int max_size)
/**
 * @brief 获取视频编码器的头部信息（如H.264的SPS/PPS）
 * @param enc_buf 输出缓冲区，用于存储头部数据
 * @param max_size 输出缓冲区的最大容量
 * @return 成功返回头部数据长度，失败返回-1
 */
```

### 4.3 执行视频帧编码操作

```c
int MppEncoder::Encode(void* mpp_buf, char* enc_buf, int max_size)
/**
 * @brief 执行视频帧编码操作
 * @param mpp_buf 输入帧数据缓冲区（YUV数据）
 * @param enc_buf 输出编码数据缓冲区
 * @param max_size 输出缓冲区最大容量
 * @return 成功返回编码数据长度，失败返回-1
 *
```



# 三、模型推理

## 1. 初始化

- 从应用上下文中获取模型信息（宽度、高度、通道数）
- 设置NMS（非极大值抑制）和置信度阈值
- 初始化RGA（Rockchip Graphic Acceleration）相关结构体
- 图像方框 im_rect

RGA结构体

```c
/* im_info definition */
typedef struct {
    void* vir_addr;                     /* virtual address */
    void* phy_addr;                     /* physical address */
    int fd;                             /* shared fd */

    int width;                          /* width */
    int height;                         /* height */
    int wstride;                        /* wstride */
    int hstride;                        /* hstride */
    int format;                         /* format */

    int color_space_mode;               /* color_space_mode */
    union {
        int global_alpha;               /* global_alpha, the default should be 0xff */
        struct {
            uint16_t alpha0;
            uint16_t alpha1;
        } alpha_bit;                    /* alpha bit(e.g. RGBA5551), 0: alpha0, 1: alpha1 */
    };
    int rd_mode;

    /* legacy */
    int color;                          /* color, used by color fill */
    im_colorkey_range colorkey_range;   /* range value of color key */
    im_nn_t nn;
    int rop_code;

    rga_buffer_handle_t handle;         /* buffer handle */
} rga_buffer_t;
```

im_rect 结构体

```c
/* Rectangle definition */
typedef struct {
    int x;        /* upper-left x */
    int y;        /* upper-left y */
    int width;    /* width */
    int height;   /* height */
} im_rect;
```



## 2. 图像预处理

- 计算输入图像到模型输入尺寸的缩放比例
- 分配内存用于存储调整大小后的图像
- 使用RGA库进行图像缩放（从原始尺寸缩放到模型输入尺寸）
- 准备RKNN输入张量

输入图像的结构体：

```c
typedef struct
{
  int width;
  int height;
  int width_stride;
  int height_stride;
  int format;
  char *virt_addr;
  int fd;
} image_frame_t;
```

缩放比例：

```c
float scale_w = (float)model_width / img->width;
float scale_h = (float)model_height / img->height;
```

准备RKNN输入张量

```c
rknn_input inputs[1];
memset(inputs, 0, sizeof(inputs));
inputs[0].index = 0;
inputs[0].type = RKNN_TENSOR_UINT8;
inputs[0].size = model_width * model_height * model_channel;
inputs[0].fmt = RKNN_TENSOR_NHWC;
inputs[0].pass_through = 0;
```

使用 RGA 进行图像缩放：

```c
// 使用RGA进行图像缩放
printf("resize with RGA!\n");
// 分配缩放缓冲区内存
resize_buf = malloc(model_width * model_height * model_channel);
// 初始化缓冲区为0
memset(resize_buf, 0, model_width * model_height * model_channel);

// 包装源图像和目标图像缓冲区
src = wrapbuffer_virtualaddr((void *)img->virt_addr, img->width, img->height, 
                             img->format, img->width_stride, img->height_stride);
dst = wrapbuffer_virtualaddr((void *)resize_buf, model_width, model_height, 
                             RK_FORMAT_RGB_888);
// 检查图像参数是否有效
ret = imcheck(src, dst, src_rect, dst_rect);
if (IM_STATUS_NOERROR != ret)
{
    printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
    return -1;
}
// 执行图像缩放操作
IM_STATUS STATUS = imresize(src, dst)
```

RKNN张量输入设置，准备输出缓冲区：

```c
rknn_inputs_set(ctx, app_ctx->io_num.n_input, inputs);
rknn_output outputs[app_ctx->io_num.n_output];
memset(outputs, 0, sizeof(outputs));
for (int i = 0; i < app_ctx->io_num.n_output; i++)
{
    outputs[i].index = i;
    outputs[i].want_float = 0;
}
```

## 3. 模型推理

执行模型推理，获取模型输出：

```c
ret = rknn_run(ctx, NULL);
ret = rknn_outputs_get(ctx, app_ctx->io_num.n_output, outputs, NULL);
```

准备输出量化和反量化参数:

```c
std::vector<float> out_scales;  // 输出缩放因子
std::vector<int32_t> out_zps;   // 输出零点偏移
for (int i = 0; i < app_ctx->io_num.n_output; ++i)
{
    out_scales.push_back(app_ctx->output_attrs[i].scale);
    out_zps.push_back(app_ctx->output_attrs[i].zp);
}
```

执行后处理：

```c
post_process((int8_t *)outputs[0].buf,  // 第一个输出缓冲区
               (int8_t *)outputs[1].buf,  // 第二个输出缓冲区
               (int8_t *)outputs[2].buf,  // 第三个输出缓冲区
               model_height, model_width,  // 模型输入尺寸
               box_conf_threshold, nms_threshold,  // 后处理阈值
               pads,  // 填充区域
               scale_w, scale_h,  // 缩放比例
               out_zps, out_scales,  // 量化参数
               detect_result);  // 检测结果输出
```

函数`int post_process(int8_t *input0, int8_t *input1, int8_t *input2, int model_in_h, int model_in_w, float conf_threshold,

​         float nms_threshold, BOX_RECT pads, float scale_w, float scale_h, std::vector<int32_t> &qnt_zps,

​         std::vector<float> &qnt_scales, detect_result_group_t *group)`

- 参数
  - @brief 目标检测后处理函数，处理模型输出并生成检测结果
  - @param input0 模型第一个输出特征图(通常对应stride 8)
  - @param input1 模型第二个输出特征图(通常对应stride 16)
  - @param input2 模型第三个输出特征图(通常对应stride 32)
  - @param model_in_h 模型输入高度
  - @param model_in_w 模型输入宽度
  - @param conf_threshold 置信度阈值，过滤低置信度检测框
  - @param nms_threshold 非极大值抑制阈值
  - @param pads 图像填充信息(用于去除预处理时的padding)
  - @param scale_w 宽度缩放比例(原始图像到模型输入的缩放)
  - @param scale_h 高度缩放比例(原始图像到模型输入的缩放)
  - @param qnt_zps 量化零点(用于反量化)
  - @param qnt_scales 量化缩放因子(用于反量化)
  - @param group 输出检测结果结构体
- 返回
  - @return 成功返回0，失败返回-1

该部分具体查看==第四节==。

释放资源：

```c
// 释放RKNN输出缓冲区
ret = rknn_outputs_release(ctx, app_ctx->io_num.n_output, outputs);

// 释放缩放缓冲区
if (resize_buf)
{
    free(resize_buf);
}
```

# 四、检测框

## 1. 目标检测后处理

### 1.1 加载标签文件

COCO文件中有80个标签，可以去看看，当然路径和标签数量都预定义好了。

```c
// 静态变量保证标签只加载一次
static int init = -1;
if (init == -1) {
    int ret = loadLabelName(LABEL_NALE_TXT_PATH, labels); // 加载类别标签文件
    if (ret < 0) {
        return -1; // 文件加载失败
    }
    init = 0; // 标记初始化完成
}
```

### 1.2 准备缓冲区

```cpp
// 清空输出缓冲区
memset(group, 0, sizeof(detect_result_group_t));

// 中间结果缓存
std::vector<float> filterBoxes;  // 存储过滤后的框坐标(x1,y1,w,h)
std::vector<float> objProbs;     // 存储对应置信度
std::vector<int> classId;         // 存储类别ID
```

### 1.3 多尺度特征图处理

```c
	// stride8处理（高分辨率检测小目标）
    int stride0 = 8;
    int grid_h0 = model_in_h / stride0;  // 特征图高度
    int grid_w0 = model_in_w / stride0;  // 特征图宽度
    int validCount0 = process(input0, (int *)anchor0, grid_h0, grid_w0, 
                            model_in_h, model_in_w, stride0, filterBoxes,
                            objProbs, classId, conf_threshold, 
                            qnt_zps[0], qnt_scales[0]); // 量化参数传入

    // stride16处理（中分辨率）
    int stride1 = 16;
    int grid_h1 = model_in_h / stride1;
    int grid_w1 = model_in_w / stride1;
    int validCount1 = process(input1, (int *)anchor1, grid_h1, grid_w1,
                            model_in_h, model_in_w, stride1, filterBoxes,
                            objProbs, classId, conf_threshold,
                            qnt_zps[1], qnt_scales[1]);

    // stride32处理（低分辨率检测大目标）
    int stride2 = 32;
    int grid_h2 = model_in_h / stride2;
    int grid_w2 = model_in_w / stride2;
    int validCount2 = process(input2, (int *)anchor2, grid_h2, grid_w2,
                            model_in_h, model_in_w, stride2, filterBoxes,
                            objProbs, classId, conf_threshold,
                            qnt_zps[2], qnt_scales[2]);
	// 合并所有尺度的有效检测数
    int validCount = validCount0 + validCount1 + validCount2;
    if (validCount <= 0) { // 无有效检测
        return 0;
    }
```

查看==第2小节==的特征图处理。

### 1.4 结果后处理

```c
	// 生成索引数组用于排序
    std::vector<int> indexArray;
    for (int i = 0; i < validCount; ++i) {
        indexArray.push_back(i);
    }
    // 按置信度降序快速排序（只排序索引）
    quick_sort_indice_inverse(objProbs, 0, validCount - 1, indexArray);

    // 获取所有唯一类别ID（使用set自动去重）
    std::set<int> class_set(std::begin(classId), std::end(classId));

    // 逐类别进行NMS处理
    for (auto c : class_set) {
        nms(validCount, filterBoxes, classId, indexArray, c, nms_threshold);
    }
```



### 1.5 格式化输出

```c
	int last_count = 0;
    group->count = 0;
    for (int i = 0; i < validCount; ++i) {
        if (indexArray[i] == -1 || last_count >= OBJ_NUMB_MAX_SIZE) {
            continue; // 跳过被NMS抑制或超过最大输出数的情况
        }

        int n = indexArray[i]; // 获取原始索引

        // 坐标转换（补偿预处理填充）
        float x1 = filterBoxes[n * 4 + 0] - pads.left;
        float y1 = filterBoxes[n * 4 + 1] - pads.top;
        float x2 = x1 + filterBoxes[n * 4 + 2]; // x1 + width
        float y2 = y1 + filterBoxes[n * 4 + 3]; // y1 + height

        // 写入输出结构体
        group->results[last_count].box.left = (int)(clamp(x1, 0, model_in_w) / scale_w);
        group->results[last_count].box.top = (int)(clamp(y1, 0, model_in_h) / scale_h);
        group->results[last_count].box.right = (int)(clamp(x2, 0, model_in_w) / scale_w);
        group->results[last_count].box.bottom = (int)(clamp(y2, 0, model_in_h) / scale_h);
        
        // 写入置信度和类别
        group->results[last_count].prop = objProbs[i];
        char *label = labels[classId[n]];
        strncpy(group->results[last_count].name, label, OBJ_NAME_MAX_SIZE);

        last_count++;
    }
    group->count = last_count; // 更新有效检测数
```

## 2. 特征图处理

```c
static int process(int8_t *input, int *anchor, int grid_h, int grid_w, 
                  int height, int width, int stride,
                  std::vector<float> &boxes, std::vector<float> &objProbs,
                  std::vector<int> &classId, float threshold,
                  int32_t zp, float scale)
/**
 * 处理单个特征图的检测结果（YOLO风格解码）
 * @param input 量化后的特征图数据(int8)
 * @param anchor 当前尺度的锚点尺寸
 * @param grid_h/w 特征图高宽
 * @param height/width 模型输入尺寸
 * @param stride 当前特征图的步长
 * @param boxes 输出框坐标(追加存储x,y,w,h)
 * @param objProbs 输出置信度(类别概率 * 框置信度)
 * @param classId 输出类别ID
 * @param threshold 置信度阈值(已量化)
 * @param zp 量化零点
 * @param scale 量化缩放系数
 * @return 有效检测框数量
 */                 
```

这段代码会遍历80个类别，比较哪个类是最大置信度。倘若只有单类别，跟阈值置信度比较即可。



## 3. 改成单一检测类型

由于类别单一了，例如我只要检测 Person 了。NMS 改成全局的，删除 loadLabelName() 的调用，只用 Labels[0] 标签。

容器 std::vector<int> classId 也移除，我们再创建个函数 process_single_class()，写成我们的逻辑。

```c
/**
 * 单类别目标检测处理函数
 * @param input 量化特征图数据(int8)
 * @param anchor 锚点尺寸数组
 * @param grid_h/w 特征图高宽
 * @param height/width 模型输入尺寸
 * @param stride 特征图步长
 * @param boxes 输出框坐标(追加x,y,w,h)
 * @param objProbs 输出置信度(仅框置信度)
 * @param threshold 置信度阈值(浮点)
 * @param zp 量化零点
 * @param scale 量化缩放系数
 * @return 有效检测框数量
 */
static int process_single_class(int8_t *input, int *anchor, int grid_h, int grid_w,
                              int height, int width, int stride,
                              std::vector<float> &boxes, std::vector<float> &objProbs,
                              float threshold, int32_t zp, float scale)
{
    int validCount = 0;
    int grid_len = grid_h * grid_w;
    int8_t thres_i8 = qnt_f32_to_affine(threshold, zp, scale); // 量化阈值

    for (int a = 0; a < 3; a++) { // 遍历3个anchor
        for (int i = 0; i < grid_h; i++) {
            for (int j = 0; j < grid_w; j++) {
                // 获取当前框的置信度（特征图第4通道）
                int8_t box_confidence = input[(PROP_BOX_SIZE * a + 4) * grid_len + i * grid_w + j];
                
                // 置信度过滤（使用量化值快速比较）
                if (box_confidence >= thres_i8) {
                    int offset = (PROP_BOX_SIZE * a) * grid_len + i * grid_w + j;
                    int8_t *in_ptr = input + offset;

                    /* YOLO坐标解码 */
                    float box_x = (deqnt_affine_to_f32(*in_ptr, zp, scale)) * 2.0 - 0.5;
                    float box_y = (deqnt_affine_to_f32(in_ptr[grid_len], zp, scale)) * 2.0 - 0.5;
                    float box_w = (deqnt_affine_to_f32(in_ptr[2 * grid_len], zp, scale)) * 2.0;
                    float box_h = (deqnt_affine_to_f32(in_ptr[3 * grid_len], zp, scale)) * 2.0;

                    // 转换为绝对坐标
                    box_x = (box_x + j) * (float)stride;
                    box_y = (box_y + i) * (float)stride;
                    box_w = box_w * box_w * (float)anchor[a * 2];
                    box_h = box_h * box_h * (float)anchor[a * 2 + 1];
                    box_x -= (box_w / 2.0); // 转左上角坐标
                    box_y -= (box_h / 2.0);

                    // 存储结果（置信度直接使用框置信度）
                    objProbs.push_back(deqnt_affine_to_f32(box_confidence, zp, scale));
                    boxes.push_back(box_x);
                    boxes.push_back(box_y);
                    boxes.push_back(box_w);
                    boxes.push_back(box_h);
                    validCount++;
                }
            }
        }
    }
    return validCount;
}
```

post_process_single() 函数：

```c
int post_process_single(int8_t *input0, int8_t *input1, int8_t *input2, 
                int model_in_h, int model_in_w,
                float conf_threshold, float nms_threshold,
                BOX_RECT pads, float scale_w, float scale_h,
                std::vector<int32_t> &qnt_zps,
                std::vector<float> &qnt_scales,
                detect_result_group_t *group)
{
    // 清空输出缓冲区
    memset(group, 0, sizeof(detect_result_group_t));

    // 中间结果缓存（不再需要classId）
    std::vector<float> filterBoxes;  // 存储框坐标(x1,y1,w,h)
    std::vector<float> objProbs;     // 存储置信度

    /******************************************
     * 多尺度特征图处理（移除classId相关参数）
     *****************************************/
    // stride8处理
    int stride0 = 8;
    int grid_h0 = model_in_h / stride0;
    int grid_w0 = model_in_w / stride0;
    int validCount0 = process_single_class(input0, (int *)anchor0, grid_h0, grid_w0, 
                                        model_in_h, model_in_w, stride0, 
                                        filterBoxes, objProbs, conf_threshold,
                                        qnt_zps[0], qnt_scales[0]);

    // stride16处理（同理修改其他尺度）
    // ...（与stride8处理方式相同）

    // stride 16
      int stride1 = 16;
      int grid_h1 = model_in_h / stride1;
      int grid_w1 = model_in_w / stride1;
      int validCount1 = 0;
      int validCount1 = process_single_class(input1, (int *)anchor1, grid_h1, grid_w1, 
                                        model_in_h, model_in_w, stride1, 
                                        filterBoxes, objProbs, conf_threshold,
                                        qnt_zps[1], qnt_scales[1]);

      // stride 32
      int stride2 = 32;
      int grid_h2 = model_in_h / stride2;
      int grid_w2 = model_in_w / stride2;
      int validCount2 = 0;
      int validCount2 = process_single_class(input2, (int *)anchor2, grid_h2, grid_w2, 
                                        model_in_h, model_in_w, stride2, 
                                        filterBoxes, objProbs, conf_threshold,
                                        qnt_zps[2], qnt_scales[2]);
    // 合并有效检测数
    int validCount = validCount0 + validCount1 + validCount2;
    if (validCount <= 0) return 0;

    /******************************************
     * 结果后处理（简化版）
     *****************************************/
    // 生成排序索引
    std::vector<int> indexArray;
    for (int i = 0; i < validCount; ++i) {
        indexArray.push_back(i);
    }

    // 按置信度排序
    quick_sort_indice_inverse(objProbs, 0, validCount - 1, indexArray);

    // 全局NMS（不再需要按类别处理）
    nms(validCount, filterBoxes, indexArray, nms_threshold);

    /******************************************
     * 输出结果（固定使用labels[0]）
     *****************************************/
    int last_count = 0;
    for (int i = 0; i < validCount; ++i) {
        if (indexArray[i] == -1 || last_count >= OBJ_NUMB_MAX_SIZE) continue;

        int n = indexArray[i];
        float x1 = filterBoxes[n * 4 + 0] - pads.left;
        float y1 = filterBoxes[n * 4 + 1] - pads.top;
        float x2 = x1 + filterBoxes[n * 4 + 2];
        float y2 = y1 + filterBoxes[n * 4 + 3];

        // 写入结果（类别名固定为labels[0]）
        group->results[last_count].box.left = (int)(clamp(x1, 0, model_in_w) / scale_w);
        group->results[last_count].box.top = (int)(clamp(y1, 0, model_in_h) / scale_h);
        group->results[last_count].box.right = (int)(clamp(x2, 0, model_in_w) / scale_w);
        group->results[last_count].box.bottom = (int)(clamp(y2, 0, model_in_h) / scale_h);
        group->results[last_count].prop = objProbs[i];
        strncpy(group->results[last_count].name, labels[0], OBJ_NAME_MAX_SIZE); // 固定类别

        last_count++;
    }
    group->count = last_count;

    return 0;
}
```



## 4. 绘制检测框

函数 `void draw_rectangle_yuv420sp(unsigned char* yuv420sp, int w, int h, int rx, int ry, int rw, int rh, unsigned int color, int thickness)`

- 参数
  - @brief 在YUV420SP格式的图像上绘制矩形框
   * @param yuv420sp 输入的YUV420SP图像数据指针
   * @param w 图像的宽度（必须为偶数）
   * @param h 图像的高度（必须为偶数）
   * @param rx 矩形框左上角x坐标（必须为偶数）
   * @param ry 矩形框左上角y坐标（必须为偶数）
   * @param rw 矩形框的宽度（必须为偶数）
   * @param rh 矩形框的高度（必须为偶数）
   * @param color 矩形框颜色（RGB格式，如0x00RRGGBB）
   * @param thickness 线宽（必须为偶数，-1表示填充整个矩形）

函数`draw_rectangle_c1(Y, w, h, w, rx, ry, rw, rh, v_y, thickness)`用于处理Y分量。

函数`draw_rectangle_c2(UV, w / 2, h / 2, w, rx / 2, ry / 2, rw / 2, rh / 2, v_uv, thickness_uv)`用于处理 UV 分量。



# 小结

本人要做的是有关人脸的，后面阅读 RetinaFace_img_demo 后，要想办法把其改成 RetinaFace_video_demo。那么就要在 yolo5_video_demo 上把 yolov5 改成 RetinaFace。

该 demo 不像隔壁海思的 demo 开辟了多线程，后面如果把 RetinaFace_video_demo 集成到 Qt 项目的时候，就要设计多线程了，当然还要添加其他的 AI 线程（未定），以及 GStreamer 进行 v4l2 视频裁剪和 RTSP 推流。
