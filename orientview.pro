TARGET = orientview
TEMPLATE = app

CONFIG += qt c++11 warn_on
QT += core gui opengl svg widgets xml

unix {
    QMAKE_CXX = $$(CXX)
    QMAKE_CXXFLAGS += -Werror
    LIBS += -lavcodec -lavformat -lavutil -lswresample -lswscale -lopencv_core -lopencv_imgproc -lopencv_photo -lopencv_video -lx264 -llsmash

    # travis-ci specific
    QMAKE_CXXFLAGS += -isystem /var/tmp/ffmpeg/include -isystem /var/tmp/opencv/include -isystem /var/tmp/x264/include -isystem /var/tmp/l-smash/include
    QMAKE_LFLAGS += -L/var/tmp/ffmpeg/lib -L/var/tmp/opencv/lib -L/var/tmp/x264/lib -L/var/tmp/l-smash/lib

    target.path = $$PREFIX/opt/orientview
    target.files = orientview data misc/icons/orientview-256x256.png
    
    desktop.path = $$PREFIX/usr/share/applications
    desktop.files = misc/linux/orientview.desktop
    
    INSTALLS += target desktop
}

win32 {
    INCLUDEPATH += include
    QMAKE_LIBDIR += lib
    CONFIG -= embed_manifest_exe
    RC_FILE = misc/windows/orientview.rc
    LIBS += avformat.lib avutil.lib avcodec.lib swscale.lib libx264.dll.lib liblsmash.lib
    debug:LIBS += opencv_core249d.lib opencv_imgproc249d.lib opencv_photo249d.lib opencv_video249d.lib
    release:LIBS += opencv_core249.lib opencv_imgproc249.lib opencv_photo249.lib opencv_video249.lib
    # data folder, *.config and debug dlls need to be copied to output folder
}

OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

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
