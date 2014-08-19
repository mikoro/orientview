TARGET = orientview
TEMPLATE = app

CONFIG += qt c++11 warn_on
QT += core gui widgets opengl svg xml
LIBS += -lavcodec -lavformat -lavutil -lswscale -lopencv_core -lopencv_imgproc -lopencv_photo -lopencv_video -lx264 -llsmash

OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

#QMAKE_CXX = clang
QMAKE_CXXFLAGS += -Werror

INCLUDEPATH += include

HEADERS  += \
    src/EncodeWindow.h \
    src/FrameData.h \
    src/GpxReader.h \
    src/InputHandler.h \
    src/MainWindow.h \
    src/MapImageReader.h \
    src/MovingAverage.h \
    src/Mp4File.h \
    src/QuickRouteReader.h \
    src/Renderer.h \
    src/RenderOffScreenThread.h \
    src/RenderOnScreenThread.h \
    src/RouteManager.h \
    src/RoutePoint.h \
    src/Settings.h \
    src/SimpleLogger.h \
    src/SplitTimeManager.h \
    src/StabilizeWindow.h \
    src/VideoDecoder.h \
    src/VideoDecoderThread.h \
    src/VideoEncoder.h \
    src/VideoEncoderThread.h \
    src/VideoStabilizer.h \
    src/VideoStabilizerThread.h \
    src/VideoWindow.h

SOURCES += \
    src/EncodeWindow.cpp \
    src/GpxReader.cpp \
    src/InputHandler.cpp \
    src/Main.cpp \
    src/MainWindow.cpp \
    src/MapImageReader.cpp \
    src/MovingAverage.cpp \
    src/Mp4File.cpp \
    src/QuickRouteReader.cpp \
    src/Renderer.cpp \
    src/RenderOffScreenThread.cpp \
    src/RenderOnScreenThread.cpp \
    src/RouteManager.cpp \
    src/Settings.cpp \
    src/SimpleLogger.cpp \
    src/SplitTimeManager.cpp \
    src/StabilizeWindow.cpp \
    src/VideoDecoder.cpp \
    src/VideoDecoderThread.cpp \
    src/VideoEncoder.cpp \
    src/VideoEncoderThread.cpp \
    src/VideoStabilizer.cpp \
    src/VideoStabilizerThread.cpp \
    src/VideoWindow.cpp

FORMS    += \
    src/EncodeWindow.ui \
    src/MainWindow.ui \
    src/StabilizeWindow.ui

RESOURCES += \
    src/OrientView.qrc
