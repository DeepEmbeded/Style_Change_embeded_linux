---
静态库和动态库的生成，以及Qt工程调用库

---

# 一、CMake简介

遇到就一个源码文件的时候，终端用 GCC 直接编译链接就行。

但是遇到多个文件时，需要使用 Makefile 进行编译链接。

我们可以通过编写 CMakeList.txt 文件来指定整个工程的编译流程，CMake会解析 CMakeList.txt文件语法，并根据当前的编译平台，生成本地化的 Makefile 和工程文件。

也就是，CMake 仅仅只是根据不同平台生成对应的 Makefile，最终还是通过 make。

# 二、使用CMake构建

这里使用RK官方的 yolov5_demo 的 CMakeList.txt 文件来学习。

## 1. 基本配置

**版本要求**

```cmake
# 设置最低 CMake 版本要求（3.4.1）
cmake_minimum_required(VERSION 3.4.1)
```

**项目名称**

```cmake
# 定义项目名称为 rknn_yolov5_demo
project(rknn_yolov5_demo)
```

**设置C++标准**

```cmake
# 设置 C++ 标准为 C++11，且必须支持
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

**安装路径**

后面使用 make install 的时候，会把库文件和头文件都打包到 install 文件夹中。

```
# 设置安装路径：项目目录下的 install/rknn_yolov5_demo_系统名称
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_yolov5_demo_${CMAKE_SYSTEM_NAME})
```

**设置库架构**

```
# 根据编译器架构设置库架构（aarch64 或 armhf）
if(CMAKE_C_COMPILER MATCHES "aarch64")
  set(LIB_ARCH aarch64)
else()
  set(LIB_ARCH armhf)
endif()
```

真要选择哪个架构，先用 cmake-gui 配置下工具链。RK用的是 aarch64，隔壁 IMX6U 用的是 armhf。

## 2. 配置相关库路径

**搜索路径**

```cmake
# 包含项目根目录作为头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}

# 设置 RKNN API 路径（根据系统名称）
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../runtime//${CMAKE_SYSTEM_NAME}/librknn_api)

# 根据平台设置 RKNN 运行时库路径
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librknnrt.so)
else()
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)
endif()

# 包含 RKNN API 和第三方库的头文件路径
include_directories(${RKNN_API_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/../3rdparty)
```

**第三方库**：OpenCV

```
# OpenCV 配置
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  # Android 平台 OpenCV 路径
  set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../3rdparty/opencv/OpenCV-android-sdk/sdk/native/jni/abi-${CMAKE_ANDROID_ARCH_ABI})
else()
  # 其他平台根据架构设置 OpenCV 路径
  if(LIB_ARCH STREQUAL "armhf")
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../3rdparty/opencv/opencv-linux-armhf/share/OpenCV)
  else()
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../3rdparty/opencv/opencv-linux-aarch64/share/OpenCV)
  endif()
endif()

# 查找 OpenCV 包（必须）
find_package(OpenCV REQUIRED)
```

**第三方库：RGA**

```
# RGA（Rockchip Graphics Acceleration）配置
set(RGA_PATH ${CMAKE_SOURCE_DIR}/../3rdparty/rga/)
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(RGA_LIB ${RGA_PATH}/libs/AndroidNdk/${CMAKE_ANDROID_ARCH_ABI}/librga.so)
else()
  # 重新检查架构（可能前面已设置）
  if(CMAKE_C_COMPILER MATCHES "aarch64")
    set(LIB_ARCH aarch64)
  else()
    set(LIB_ARCH armhf)
  endif()
  set(RGA_LIB ${RGA_PATH}/libs/Linux//gcc-${LIB_ARCH}/librga.so)
endif()
include_directories(${RGA_PATH}/include
```

**第三方库**：MPP

```
# MPP（Media Process Platform）配置
set(MPP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/mpp)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(MPP_LIBS ${MPP_PATH}/${CMAKE_SYSTEM_NAME}/${LIB_ARCH}/librockchip_mpp.so)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(MPP_LIBS ${MPP_PATH}/${CMAKE_SYSTEM_NAME}/${CMAKE_ANDROID_ARCH_ABI}/libmpp.so)
endif()
include_directories(${MPP_PATH}/include)
```

**第三方库：ZLMediaKit** 

```
# ZLMediaKit 配置
set(ZLMEDIAKIT_PATH ${CMAKE_SOURCE_DIR}/../3rdparty/zlmediakit)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  include_directories(${ZLMEDIAKIT_PATH}/include)
  set(ZLMEDIAKIT_LIBS ${ZLMEDIAKIT_PATH}/${LIB_ARCH}/libmk_api.so)
endif()

# 如果 ZLMediaKit 库存在，添加视频 RTSP 构建定义
if(ZLMEDIAKIT_LIBS)
  add_definitions(-DBUILD_VIDEO_RTSP)
endif()
```

**控制搜索路径**

```
# 设置安装 RPATH
set(CMAKE_INSTALL_RPATH "lib")

# 包含项目头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)
```

## 3. 生成可执行文件

```
##################这部分没有生成库文件，只生成可执行文件########
# 添加主可执行文件 rknn_yolov5_demo
add_executable(rknn_yolov5_demo
  src/main.cc
  src/preprocess.cc
  src/postprocess.cc
)

# 链接主可执行文件所需的库
target_link_libraries(rknn_yolov5_demo
  ${RKNN_RT_LIB}
  ${RGA_LIB}
  ${OpenCV_LIBS}
)
```



## 4. 生成库文件

```cmake
# 如果 MPP 库存在，添加视频相关的静态库
if(MPP_LIBS)
  add_library(rknn_yolov5_lib SHARED/STATIC 
    src/main_video.cc
    src/postprocess.cc
    utils/mpp_decoder.cpp
    utils/mpp_encoder.cpp
    utils/drawing.cpp
  )
  # 链接静态库所需的库
  target_link_libraries(rknn_yolov5_lib
    ${RKNN_RT_LIB}
    ${RGA_LIB}
    ${OpenCV_LIBS}
    ${MPP_LIBS}
    ${ZLMEDIAKIT_LIBS}
  )
endif()

# 如果 MPP 库存在，安装动态库/静态库和头文件
if(MPP_LIBS)
  install(TARGETS rknn_yolov5_lib
  LIBRARY DESTINATION lib   # 动态库 (.so)
  ARCHIVE DESTINATION lib   # 静态库 (.a，可选)
  RUNTIME DESTINATION bin   # Windows DLL (可选)
)
# 安装头文件
install(DIRECTORY include/ DESTINATION include)
endif()
```

其中 add_library 中的 SHARED 以及 STATIC 分别表示生产库的类型：动态和静态。



## 5. 安装

```cmake
# 安装主可执行文件到当前目录
install(TARGETS rknn_yolov5_demo DESTINATION ./)

# 安装 RKNN 和 RGA 库到 lib 目录
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
install(PROGRAMS ${RGA_LIB} DESTINATION lib)

# 安装模型文件
install(DIRECTORY model/${TARGET_SOC} DESTINATION ./model)
file(GLOB IMAGE_FILES "model/*.jpg")
file(GLOB LABEL_FILE "model/*.txt")
install(FILES ${IMAGE_FILES} DESTINATION ./model/)
install(FILES ${LABEL_FILE} DESTINATION ./model/)


# 如果 ZLMediaKit 库存在，安装到 lib 目录
if(ZLMEDIAKIT_LIBS)
  install(PROGRAMS ${ZLMEDIAKIT_LIBS} DESTINATION lib)
endif()
```

# 三、动态和静态的场景

**动态库**

移植程序的时候，记得把相关的动态库文件.so 放到指定路径中。

有时候，可能你板端的动态库和编译时使用的版本不兼容，会报错。



**静态库**

所有代码（包括依赖的静态库）已编译到可执行文件中，板端只需有可执行文件本身，无需安装任何库。

但是体积很大，占用更多存储空间。



# 四、Qt 添加库文件

![image-20250421161126452](C:\Users\13227\AppData\Roaming\Typora\typora-user-images\image-20250421161126452.png)

![image-20250421161143305](C:\Users\13227\AppData\Roaming\Typora\typora-user-images\image-20250421161143305.png)

然后可以把上面 CMake 生成过的库文件和头文件拷贝到 Qt项目下的 lib 和 include，自己创建即可。

![image-20250421161328596](C:\Users\13227\AppData\Roaming\Typora\typora-user-images\image-20250421161328596.png)

平台看自己，嵌入式肯定选 Linux

![image-20250421161409211](C:\Users\13227\AppData\Roaming\Typora\typora-user-images\image-20250421161409211.png)