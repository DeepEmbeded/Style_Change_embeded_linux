---
yolov5使用npu多线程
---

# 一、主函数

## 1. **线程池和任务数组**

```c
struct thread_pool thpol;
struct task task[THREAD_COUNT];
```

创建了线程池以及任务数组，我们看看定义是什么样的：

```c
    struct task {
        void* (*task_fun)(void* p);
        void* user_data;
        struct task* prev, * next;
    };

    struct work_task {
        pthread_t id;
        int termination;
        struct thread_pool* thread_pool;
        struct work_task* prev, * next;
    };

    struct thread_pool {
        struct work_task* work_task_queue;//��������
        struct task* tasks;
        pthread_cond_t cond;
        pthread_mutex_t mutex;
    };
```

## 2. **定义需要的变量**

```cpp
clock_t startTime,endTime;
char *imgName = "bus.jpg";
Mat image, orig_img[THREAD_COUNT];
detect_result_group_t tdetect_result_group; 
```

定义了开始结束时间、需要处理的图片、以及OpenCV的帧格式Mat、检测结果的队列。

该代码的检测结果就64，上次看的源码是80个。

## 3. **初始化各个线程的模型**

```c
	int ret = 0;
    int frame_num = 0;
    int i = 0;
    for(int i = 0;i < THREAD_COUNT;i++){
        tdetect[i].rknn_envs_init("./yolov5s.rknn", i % 3);

    }

    ret = thread_poll_create(&thpol, THREAD_COUNT);
    if (ret != THREAD_COUNT) {
        printf("poll creat error\n");
        return -1;
    }
```



## 4. **加载图片开始计时**

```c
	// 加载输入图像
    image = imread(imgName, 1);
    if(image.empty()) {
        printf("could not load image %s\n", imgName);
        return -1;
    }
    
    startTime = clock();  // 计时开始
    int frame_rate = 0;  // 帧率计数器
```



## 5. **多线程运作**

```c
	// 主循环
    while(1) {
        // 计算当前任务索引（循环使用）
        i = frame_num % THREAD_COUNT;
        
        // 设置任务参数
        tdetect[i].detect_img = image.clone();  // 使用clone()避免数据竞争
        task[i].task_fun = counter;  // 设置任务函数
        task[i].user_data = &tdetect[i];  // 设置任务数据
        
        // 将任务加入线程池
        thread_poll_push_task(&thpol, &task[i]);
        
        frame_num++;  // 更新帧计数
        frame_rate++;  // 更新帧率计数
        
        // 计算帧率
        endTime = clock();
        double t = (double)(endTime - startTime) / CLOCKS_PER_SEC * 1000;
        if (t >= 1000) {  // 每1秒输出一次帧率
            cout << "frame_rate: " << frame_rate << endl;
            frame_rate = 0;
            startTime = clock();  // 重置计时器
        }
    }
```



# 二、检测类

文件detect.h 定义了 detect 类：

```c
class detect{
 
public:
    rknn_context             ctx;
    rknn_sdk_version         version;
    rknn_input_output_num    io_num;
    struct timeval           start_time, stop_time;
    Mat                      detect_img; 
    size_t                   actual_size = 0;
    int                      img_width;
    int                      img_height;
    int                      img_channel;
    const float              nms_threshold      = NMS_THRESH;
    const float              box_conf_threshold = BOX_THRESH;
    rga_buffer_t             src;
    rga_buffer_t             dst;
    im_rect                  src_rect;
    im_rect                  dst_rect;
    char                     *model_path;
    
                             detect();
                            ~detect();
    int                      rknn_envs_init(const char* model_path, int npu_index);
    int                      rknn_envs_free();
    int                      detect_image(Mat &orig_img, detect_result_group_t *detect_result_group);
    int                      draw_results(Mat &orig_img, detect_result_group_t *detect_result_group);
 
 
private:
    int                      model_data_size;
    unsigned char            *model_data;
 
    unsigned char*           load_data(FILE* fp, size_t ofst, size_t sz);
    unsigned char*           load_model(const char* filename, int* model_size);
    
};
```

经过上个demo源码的阅读，其实就是把上个demo文件的几个函数和变量封装了起来。

当然作者为了匹配NPU线程和OPenCV图像处理也重新写了几个函数：

## 1. **初始化rknn模型的运行环境**

其中NPU核心绑定需要通过官方的rknn_api.h的 rknn_core_mask 结构体。

```c
int detect::rknn_envs_init(const char* model_path, int npu_index) 
{
    int ret = 0;    // 返回值变量
    
    /* --- 第一步：加载RKNN模型 --- */
    model_data_size = 0;  // 初始化模型数据大小
    // 加载模型文件到内存
    model_data = load_model(model_path, &model_data_size);
    // 初始化RKNN上下文
    ret = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return -1;  // 错误码-1表示模型初始化失败
    }

    /* --- 第二步：设置NPU核心绑定 --- */
    rknn_core_mask core_mask = RKNN_NPU_CORE_0;  // 默认使用核心0
    // 根据输入参数选择不同的NPU核心
    if (npu_index == 0)
        core_mask = RKNN_NPU_CORE_0;
    else if(npu_index == 1)
        core_mask = RKNN_NPU_CORE_1;
    else if(npu_index == 2)
        core_mask = RKNN_NPU_CORE_2;
    // 应用核心绑定设置
    ret = rknn_set_core_mask(ctx, core_mask);
    if (ret < 0){
        printf("rknn_init core error ret=%d\n", ret);
        return -2;  // 错误码-2表示核心绑定失败
    }

    /* --- 第三步：查询SDK版本信息 --- */
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if (ret < 0) {
        printf("rknn_query RKNN_QUERY_SDK_VERSION error ret=%d\n", ret);
        return -3;  // 错误码-3表示版本查询失败
    }
    // 调试时可打印版本信息
    // printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);
 
    /* --- 第四步：查询模型输入输出信息 --- */
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0) {
        printf("rknn_init RKNN_QUERY_IN_OUT_NUM error ret=%d\n", ret);
        return -4;  // 错误码-4表示IO查询失败
    }

    return 0;  // 初始化成功
}
```

## 2. **图像识别**

```c
int detect::detect_image(Mat &orig_img, detect_result_group_t *detect_result_group) 
{
    int ret = 0;
    void* resize_buf = nullptr;  // 图像缩放缓冲区
    rknn_tensor_attr input_attrs[io_num.n_input];  // 模型输入属性

    /* --- 图像预处理阶段 --- */
    Mat img;   // 用于NPU推理的图像
    Mat tImg;  // 临时转换图像
    // 1. BGR转RGB（OpenCV默认BGR，模型需要RGB）
    cvtColor(orig_img, tImg, cv::COLOR_BGR2RGB);
    // 2. 缩放至模型输入尺寸640x640（保持原始比例）
    resize(tImg, img, Size(640, 640), 0, 0, cv::INTER_NEAREST);

    img_width  = img.cols;  // 记录处理后图像宽
    img_height = img.rows;  // 记录处理后图像高

    /* --- 查询模型输入输出属性 --- */
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret < 0) {
            printf("rknn_init error ret=%d\n", ret);
            return -1;
        }
    }

    rknn_tensor_attr output_attrs[io_num.n_output];  // 模型输出属性
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
    }

    /* --- 解析模型输入格式 --- */
    int channel = 3;  // 默认RGB三通道
    int width   = 0;
    int height  = 0;
    // 判断数据布局格式（NCHW或NHWC）
    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
        channel = input_attrs[0].dims[1];
        width   = input_attrs[0].dims[2];
        height  = input_attrs[0].dims[3];
    } else {
        width   = input_attrs[0].dims[1];
        height  = input_attrs[0].dims[2];
        channel = input_attrs[0].dims[3];
    }

    /* --- 准备模型输入数据 --- */
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index        = 0;  // 输入索引
    inputs[0].type         = RKNN_TENSOR_UINT8;  // 数据类型
    inputs[0].size         = width * height * channel;  // 数据大小
    inputs[0].fmt          = RKNN_TENSOR_NHWC;  // 数据布局
    inputs[0].pass_through = 0;  // 是否透传

    // 处理尺寸不匹配的情况
    if (img_width != width || img_height != height) {
        resize_buf = malloc(height * width * channel);  // 分配缩放缓冲区
        memset(resize_buf, 0x00, height * width * channel);

        // 使用Rockchip图像处理库进行高效缩放
        src = wrapbuffer_virtual_addr((void*)img.data, img_width, img_height, RK_FORMAT_RGB_888);
        dst = wrapbuffer_virtual_addr((void*)resize_buf, width, height, RK_FORMAT_RGB_888);
        ret = imcheck(src, dst, src_rect, dst_rect);
        if (IM_STATUS_NOERROR != ret) {
            printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
            return -1;
        }
        inputs[0].buf = resize_buf;  // 使用缩放后的图像
    } else {
        inputs[0].buf = (void*)img.data;  // 直接使用原图数据
    }

    /* --- 执行模型推理 --- */
    gettimeofday(&start_time, NULL);  // 记录开始时间
    // 设置模型输入
    rknn_inputs_set(ctx, io_num.n_input, inputs);
    
    // 准备输出缓冲区
    rknn_output outputs[io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < io_num.n_output; i++) {
        outputs[i].want_float = 0;  // 输出为量化后的整数
    }

    // 执行推理
    ret = rknn_run(ctx, NULL);
    // 获取输出结果
    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
    gettimeofday(&stop_time, NULL);  // 记录结束时间

    /* --- 后处理阶段 --- */
    // 计算缩放比例（模型输入 vs 原始图像）
    float scale_w = (float)width / orig_img.cols;
    float scale_h = (float)height / orig_img.rows;

    // 收集输出量化和反量化参数
    std::vector<float> out_scales;
    std::vector<int32_t> out_zps;
    for (int i = 0; i < io_num.n_output; ++i) {
        out_scales.push_back(output_attrs[i].scale);  // 量化尺度
        out_zps.push_back(output_attrs[i].zp);        // 量化零点
    }

    // 执行后处理（解析检测框、置信度、类别等）
    post_process(
        (int8_t*)outputs[0].buf,  // 输出1数据
        (int8_t*)outputs[1].buf,  // 输出2数据
        (int8_t*)outputs[2].buf,  // 输出3数据
        height, width,
        box_conf_threshold,  // 置信度阈值
        nms_threshold,       // NMS阈值
        scale_w, scale_h,    // 缩放比例
        out_zps, out_scales, // 量化参数
        detect_result_group   // 输出检测结果
    );

    // 释放输出缓冲区
    ret = rknn_outputs_release(ctx, io_num.n_output, outputs);

    // 释放缩放缓冲区
    if (resize_buf) {
        free(resize_buf);
        resize_buf = nullptr;
    }

    return 0;
}
```



## 3. **绘制方框在原图上**

```c
int detect::draw_results(Mat &orig_img, detect_result_group_t *detect_result_group)
{
    char text[256];  // 用于显示类别和置信度的文本缓冲区
    
    // 遍历所有检测结果
    for (int i = 0; i < detect_result_group->count; i++) {
        // 获取当前检测结果
        detect_result_t* det_result = &(detect_result_group->results[i]);
        
        /* --- 准备显示文本 --- */
        // 格式化文本：类别名称 + 置信度百分比（如 "person 98.5%"）
        sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);
        
        /* --- 解析边界框坐标 --- */
        int x1 = det_result->box.left;    // 左上角x坐标
        int y1 = det_result->box.top;     // 左上角y坐标
        int x2 = det_result->box.right;   // 右下角x坐标
        int y2 = det_result->box.bottom;   // 右下角y坐标

        /* --- 绘制边界框 --- */
        // 参数说明：
        // orig_img: 原始图像
        // Point(x1,y1): 矩形左上角坐标
        // Point(x2,y2): 矩形右下角坐标
        // Scalar(255,0,0,255): 蓝色框（BGR格式），线宽3像素
        rectangle(orig_img, cv::Point(x1, y1), cv::Point(x2, y2), 
                 cv::Scalar(255, 0, 0, 255), 3);

        /* --- 绘制类别标签 --- */
        // 参数说明：
        // orig_img: 原始图像
        // text: 显示文本
        // Point(x1,y1+12): 文本位置（Y坐标下移12像素避免重叠）
        // FONT_HERSHEY_SIMPLEX: 字体类型
        // 0.5: 字体缩放比例
        // Scalar(0,0,0): 黑色文本
        putText(orig_img, text, cv::Point(x1, y1 + 12), 
               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }

    // 调试时可取消注释以下内容实时显示
    // imshow("Detection Results", orig_img);
    // cv::waitKey(1);  // 保持1ms用于图像刷新

    return 0;  // 成功返回
}
```



# 三、线程池

定义了5个函数: 线程池创建，任务加入线程池、销毁线程、线程回调函数、任务线程。

```c
int thread_poll_create(struct thread_pool* thread_pool, int thread_num);
int thread_poll_push_task(struct thread_pool* thread_pool, struct task* task);
void thread_destroy(struct thread_pool* thread_pool);
void* thread_callback(void* arg);
struct task* get_task(struct work_task* worker);
```

我们直接用人家写好的也行，可以了解了解代码逻辑。



## 1. 线程池创建

```c
int thread_poll_create(struct thread_pool* thread_pool, int thread_num) 
{
    // 参数校验：线程数至少为1
    if (thread_num < 1) thread_num = 1;
    
    // 初始化线程池结构体
    memset(thread_pool, 0, sizeof(struct thread_pool));

    /* --- 初始化条件变量 --- */
    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;  // 静态初始化条件变量
    memcpy(&thread_pool->cond, &blank_cond, sizeof(pthread_cond_t));
    /* 注：
     * 直接使用memcpy复制初始化器是线程安全的
     * 比pthread_cond_init()更简洁
     */

    /* --- 初始化互斥锁 --- */
    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;  // 静态初始化互斥锁
    memcpy(&thread_pool->mutex, &blank_mutex, sizeof(pthread_mutex_t));
    /* 注：
     * 与条件变量同理，避免显式调用init函数
     * 这种写法在Linux平台安全，但非POSIX标准
     */

    /* --- 创建工作线程 --- */
    int idx = 0;
    for (idx = 0; idx < thread_num; idx++) {
        // 分配工作线程结构体
        struct work_task* worker = (struct work_task*)malloc(sizeof(struct work_task));
        if (worker == NULL) {
            perror("worker malloc error");
            return idx;  // 返回已创建的线程数
        }
        memset(worker, 0, sizeof(struct work_task));
        
        // 设置线程池指针
        worker->thread_pool = thread_pool;

        // 创建线程（绑定回调函数thread_callback）
        int ret = pthread_create(&worker->id, NULL, thread_callback, worker);
        if (ret) {
            perror("pthread_create error");
            free(worker);  // 创建失败时释放内存
            return idx;    // 返回已创建的线程数
        }

        // 将工作线程加入链表（LL_ADD是链表操作宏）
        LL_ADD(worker, thread_pool->work_task_queue);
    }
    return idx;  // 返回实际创建的线程数
}
```



## 2. 任务加入线程池

```c
int thread_poll_push_task(struct thread_pool* thread_pool, struct task* task) 
{
    /* --- 临界区开始：加锁保护共享资源 --- */
    pthread_mutex_lock(&thread_pool->mutex);
    
    /* --- 任务入队操作 --- */
    // LL_ADD是链表操作宏，将新任务添加到任务队列头部
    // 等效于：
    // task->next = thread_pool->tasks;
    // if(thread_pool->tasks) thread_pool->tasks->prev = task;
    // thread_pool->tasks = task;
    LL_ADD(task, thread_pool->tasks);
    
    /* --- 唤醒工作线程 --- */
    // 发送条件信号，唤醒可能正在等待的线程
    // 注意：必须在持有锁的情况下调用pthread_cond_signal
    pthread_cond_signal(&thread_pool->cond);
    
    /* --- 临界区结束：释放锁 --- */
    pthread_mutex_unlock(&thread_pool->mutex);
    
    return 0;  // 成功返回
}
```



## 3. 线程销毁

```c
void thread_destroy(struct thread_pool* thread_pool) 
{
    /* --- 第一阶段：设置终止标志 --- */
    struct work_task* worker = NULL;
    // 遍历所有工作线程，设置终止标志位
    for (worker = thread_pool->work_task_queue; worker != NULL; worker = worker->next) {
        worker->termination = 1;  // 原子操作更安全（建议改进点）
    }

    /* --- 第二阶段：唤醒所有线程 --- */
    pthread_mutex_lock(&thread_pool->mutex);
    // 广播条件变量，唤醒所有等待中的线程
    pthread_cond_broadcast(&thread_pool->cond);
    pthread_mutex_unlock(&thread_pool->mutex);

    /* --- 第三阶段：实际回收资源（应在外部实现）--- */
    // 注意：此处缺少以下关键操作：
    // 1. pthread_join 等待线程退出
    // 2. 释放worker结构体内存
    // 3. 销毁mutex和cond
}
```



## 4. 获取任务

```c
struct task* get_task(struct work_task* worker)
{
    while (1) {  // 持续尝试获取任务
        /* --- 进入临界区：加锁保护共享资源 --- */
        pthread_mutex_lock(&worker->thread_pool->mutex);

        /* --- 等待条件满足 --- */
        // 当任务队列为空且线程未被终止时，进入等待状态
        while (worker->thread_pool->tasks == NULL) {  // 使用while防止虚假唤醒
            // 检查终止标志
            if (worker->termination) break;  // 需要终止时跳出等待循环
            // 条件变量等待（自动释放锁并在唤醒时重新获取）
            pthread_cond_wait(&worker->thread_pool->cond, 
                             &worker->thread_pool->mutex);
        }

        /* --- 终止检查 --- */
        if (worker->termination) {
            pthread_mutex_unlock(&worker->thread_pool->mutex);
            return NULL;  // 返回NULL表示线程应当退出
        }

        /* --- 任务提取 --- */
        struct task* task = worker->thread_pool->tasks;  // 获取队列头部任务
        if (task) {
            // 从队列中移除该任务（LL_REMOVE是链表操作宏）
            LL_REMOVE(task, worker->thread_pool->tasks);
        }

        /* --- 退出临界区 --- */
        pthread_mutex_unlock(&worker->thread_pool->mutex);

        /* --- 返回有效任务 --- */
        if (task != NULL) {
            return task;  // 返回获取到的任务
        }
        // 如果task为NULL则继续循环
    }
}
```



## 5. 线程回调函数

```c
void* thread_callback(void* arg) 
{
    // 获取工作线程结构体指针
    struct work_task* worker = (struct work_task*)arg;
    
    // 线程主循环
    while (1) {
        /* --- 获取待处理任务 --- */
        struct task* task = get_task(worker);  // 阻塞式获取任务
        
        /* --- 终止条件检查 --- */
        if (task == NULL) {  // NULL表示需要终止线程
            // 释放worker结构体内存
            free(worker);
            // 线程退出（携带状态信息）
            pthread_exit((void*)"thread termination\n");
        }
        
        /* --- 执行任务 --- */
        task->task_fun(task);  // 调用任务函数指针
    }
    
    // 理论上不会执行到此处
    return NULL;
}
```





