CONFIG += qt release
QT += core gui opengl svg xml
HEADERS += src/*.h
SOURCES += src/*.cpp
FORMS += src/*.ui
RESOURCES += src/*.qrc
LIBS += -lavcodec -lavformat -lavutil -lswscale -lopencv_core -lopencv_imgproc -lopencv_photo -lopencv_video -lx264 -llsmash
INCLUDEPATH += include
QMAKE_CXXFLAGS += -std=c++11 -O3
OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
TARGET = orientview
