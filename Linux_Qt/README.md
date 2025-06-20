# 一、文件说明

```
├─install
│  └─rk3588_linux_aarch64
│      ├─include
│      │  └─utils
│      ├─lib				//相关依赖库
│      └─model				//三个模型:yolov5, whisper_decoder, whisper_encoder

├─worker.h					//摄像头采集线程类, 包括原始帧推流
├─worker.cpp	

├─yolov5detector.h			//yolov5 检测线程类
├─yolov5detector.h

├─micrecorder.h				//麦克风音频采集线程类
├─micrecorder.cpp	

├─whisper_rknn.h			//whisper模型推理类
├─whisper_rknn.cpp	

├─whisperworker.h			//whisper推理线程类
├─whisperworker.cpp

├─mqttreceiver.h
├─mqttreceiver.cpp			//mqtt订阅类

├─serialportmanager.h
├─serialportmanager.cpp		//Uart串口发送类

├─widget.h
├─widget.cpp				//主窗口
├─widget.ui
```



# 二、