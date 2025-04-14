# Style_Change_embeded_linux
**用到的网络协议**：RTSP（必须）、MQTT（如果需要物联）、UDP（弹幕的传输）

## 一、板端环境配置

**要求**

- U-boot、Kernel 以及 buildroot 搭建板端环境
- IMX摄像头、7寸USB触摸屏HDMI显示、WIFI或4G模块 等外设进行驱动适配

>李永康负责

## 二、板端视频流

**要求**

- Qt 设计 UI 控件并集成视频流的 VI、VPSS、VEVC、VO等操作
- 设计多线程：视频通道线程和AI推理线程
- RTSP推流，可以用Live555现成框架

>彭严瀚负责

## 三、AI板端部署

**要求**

- AI模型训练
- AI模型量化参数
- AI模型转化
- AI模型部署
- 编写模型推理线程

>彭严瀚负责

## 四、服务端

**要求**

- RTSP拉流，Live555现成框架
- WebRTC 视频直播
- 提取弹幕，与板端进行UDP网络通信

>余建负责

## 五、板端语音模块

**要求**

- 获取服务器的弹幕，并进行语音输出
- 需要用到UDP网络通信

>李永康负责



## 六、官方资料

[ELF 2 高性能嵌入式 AI学习 | ElfBoard 官网-嵌入式Linux开发板/学习板-让嵌入式学习释放无限可能](https://www.elfboard.com/information/detail.html?id=7)



