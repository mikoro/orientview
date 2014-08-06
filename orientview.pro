CONFIG += qt release
QT += core gui opengl xml
HEADERS += src/*.h
SOURCES += src/*.cpp
FORMS += src/*.ui
INCLUDEPATH += include
QMAKE_CXXFLAGS += -std=c++11 -O3
OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
TARGET = orientview
