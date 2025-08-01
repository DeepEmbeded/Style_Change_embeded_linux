#-------------------------------------------------
#
# Project created by QtCreator 2025-05-27T09:05:10
#
#-------------------------------------------------

QT += core gui widgets
QT += serialport
QT += mqtt network

CONFIG += debug
CONFIG -= release

TARGET = RTMP_Yolo_widget
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += WEBRTC_POSIX

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11


# 交叉编译器 + sysroot 路径
QMAKE_CC = /home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/bin/aarch64-buildroot-linux-gnu-gcc
QMAKE_CXX = /home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/bin/aarch64-buildroot-linux-gnu-g++
QMAKE_LINK = $$QMAKE_CXX
QMAKE_AR = /home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/bin/aarch64-buildroot-linux-gnu-ar cqs

# 设置 sysroot 路径
QMAKE_SYSROOT = /home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/aarch64-buildroot-linux-gnu/sysroot

# 设置 pkg-config
PKG_CONFIG_SYSROOT_DIR = $$QMAKE_SYSROOT
PKG_CONFIG_LIBDIR = $$QMAKE_SYSROOT/usr/lib/pkgconfig
PKG_CONFIG_PATH = $$PKG_CONFIG_LIBDIR

# 添加 GStreamer 支持（用 pkg-config 自动找头文件/链接库）
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0 gobject-2.0
LIBS += -lgstapp-1.0

SOURCES += \
        main.cpp \
        widget.cpp \
    yolov5detector.cpp \
    worker.cpp \
    serialportmanager.cpp \
    mqttreceiver.cpp \
#    detectorpool.cpp
    whisperworker.cpp \
    whisper_rknn.cpp \
    micrecorder.cpp \
    llmworker.cpp \
    threadutils.cpp \
    avstreamer.cpp


HEADERS += \
        widget.h \
    yolov5detector.h \
    worker.h \
    serialportmanager.h \
    mqttreceiver.h \
#    detectorpool.h
    whisperworker.h \
    whisper_rknn.h \
    micrecorder.h \
    safequeue.h \
    framebuffer.h \
    llmworker.h \
    threadutils.h \
    logger.h \
    avstreamer.h

FORMS += \
        widget.ui

QMAKE_LFLAGS += -Wl,-rpath-link,/home/elf/3rdparty/opencv-4.9.0/install/lib

# 使用 WebRTC Vad
INCLUDEPATH += /home/elf/Linux/Qt/pcm_1/install/webrtcvad/include
LIBS += -L$$PWD/install/webrtcvad/lib/ -lwebrtcVad

# 使用 OpenCV 4.6.0 的完整路径（推荐）
INCLUDEPATH += /home/elf/3rdparty/opencv-4.9.0/install/include/opencv4
LIBS += -L/home/elf/3rdparty/opencv-4.9.0/install/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_imgcodecs \
        -lopencv_videoio \
        -lopencv_tracking
LIBS += -L/home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/aarch64-buildroot-linux-gnu/sysroot/usr/lib  -lasound

unix:!macx:
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lrknn_yolov5_demo
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lrga
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lrknnrt
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -limagedrawing
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lfileutils
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -limageutils
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lrknn_whisper_demo
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -laudioutils
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lfftw3f
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lsndfile
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lrkllmrt
            LIBS += -L$$PWD/install/rk3588_linux_aarch64/lib/ -lgomp

INCLUDEPATH += $$PWD/install/rk3588_linux_aarch64/include/utils
DEPENDPATH += $$PWD/install/rk3588_linux_aarch64/include/utils


INCLUDEPATH += /home/elf/Linux/qtmqtt-5.15.2/build/include
LIBS += -L/home/elf/Linux/qtmqtt-5.15.2/build/lib -lQt5Mqtt


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#target.path = /home/elf/2025_6_20
#INSTALLS += target

DISTFILES += \
    webrtc_vad/LICENSE
