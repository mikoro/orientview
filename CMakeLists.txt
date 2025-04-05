cmake_minimum_required(VERSION 3.16)

if (WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg-ports")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(orientview VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui OpenGL Svg Widgets Xml REQUIRED)
find_package(OpenCV REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(lsmash CONFIG REQUIRED)

pkg_check_modules(X264 REQUIRED IMPORTED_TARGET x264)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
include(GNUInstallDirs)

add_executable(orientview
  src/EncodeWindow.cpp src/EncodeWindow.h src/EncodeWindow.ui
  src/FrameData.h
  src/GpxReader.cpp src/GpxReader.h
  src/InputHandler.cpp src/InputHandler.h
  src/Main.cpp
  src/MainWindow.cpp src/MainWindow.h src/MainWindow.ui
  src/MapImageReader.cpp src/MapImageReader.h
  src/MovingAverage.cpp src/MovingAverage.h
  src/Mp4File.cpp src/Mp4File.h
  src/QuickRouteReader.cpp src/QuickRouteReader.h
  src/Renderer.cpp src/Renderer.h
  src/RenderOffScreenThread.cpp src/RenderOffScreenThread.h
  src/RenderOnScreenThread.cpp src/RenderOnScreenThread.h
  src/RouteManager.cpp src/RouteManager.h
  src/RoutePoint.h
  src/Settings.cpp src/Settings.h
  src/SimpleLogger.cpp src/SimpleLogger.h
  src/SplitsManager.cpp src/SplitsManager.h
  src/StabilizeWindow.cpp src/StabilizeWindow.h src/StabilizeWindow.ui
  src/VideoDecoder.cpp src/VideoDecoder.h
  src/VideoDecoderThread.cpp src/VideoDecoderThread.h
  src/VideoEncoder.cpp src/VideoEncoder.h
  src/VideoEncoderThread.cpp src/VideoEncoderThread.h
  src/VideoStabilizer.cpp src/VideoStabilizer.h
  src/VideoStabilizerThread.cpp src/VideoStabilizerThread.h
  src/VideoWindow.cpp src/VideoWindow.h
)

qt_add_resources(RESOURCES src/OrientView.qrc)
target_sources(orientview PRIVATE ${RESOURCES})

target_include_directories(orientview PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(orientview PRIVATE
  ${FFMPEG_LIBRARY_DIRS}
)

target_link_libraries(orientview PRIVATE
  Qt5::Core
  Qt5::Gui
  Qt5::OpenGL
  Qt5::Svg
  Qt5::Widgets
  Qt5::Xml
  PkgConfig::X264
  lsmash::lsmash
  ${OpenCV_LIBS}
  ${FFMPEG_LIBRARIES}
)

add_custom_command(TARGET orientview POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:orientview>/data)

install(TARGETS orientview
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_BINDIR})
