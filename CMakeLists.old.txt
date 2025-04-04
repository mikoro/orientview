#############################################################################################
#   This is a CMake mirror of Qmake project file (orientview.pro)                           #
#   Orientview.pro is the primary source of truth for the project.                          #
#   Any changes to building should first be made in the .pro file and then mirrored here.   #
#############################################################################################

cmake_minimum_required(VERSION 3.16)
project(orientview VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui OpenGL Svg Widgets Xml)

add_executable(orientview WIN32 MACOSX_BUNDLE
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
        src/RenderOffScreenThread.cpp src/RenderOffScreenThread.h
        src/RenderOnScreenThread.cpp src/RenderOnScreenThread.h
        src/Renderer.cpp src/Renderer.h
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
target_link_libraries(orientview PRIVATE
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::Svg
        Qt::Widgets
        Qt::Xml
)


# Resources:
set(OrientView_resource_files
        "src/../misc/icons/document-open.svg"
        "src/../misc/icons/document-save-as.svg"
        "src/../misc/icons/edit-clear.svg"
        "src/../misc/icons/help.svg"
        "src/../misc/icons/media-playback-start.svg"
        "src/../misc/icons/orientview.ico"
        "src/../misc/icons/system-log-out.svg"
        "src/../misc/icons/video-x-generic.svg"
)

qt_add_resources(RESOURCES src/OrientView.qrc)
target_sources(orientview PRIVATE ${RESOURCES})

if(UNIX)
    target_include_directories(orientview PRIVATE
            /usr/include/opencv4
    )

    target_link_libraries(orientview PRIVATE
            avcodec
            avformat
            avutil
            lsmash
            opencv_core
            opencv_imgproc
            opencv_video
            swresample
            swscale
            x264
    )
endif()

if(WIN32)
    target_include_directories(orientview PRIVATE
            include
    )

    target_link_libraries(orientview PRIVATE
            avcodec.lib
            avformat.lib
            avutil.lib
            liblsmash.lib
            libx264.dll.lib
            swscale.lib
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug AND WIN32)
    target_link_libraries(orientview PRIVATE
            opencv_core249d.lib
            opencv_imgproc249d.lib
            opencv_photo249d.lib
            opencv_video249d.lib
    )
endif()

if(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    target_link_libraries(orientview PRIVATE
            opencv_core249.lib
            opencv_imgproc249.lib
            opencv_photo249.lib
            opencv_video249.lib
    )
endif()

if(APPLE)
    target_link_libraries(orientview PRIVATE
            avcodec
            avformat
            avutil
            lsmash
            opencv_core
            opencv_imgproc
            opencv_photo
            opencv_video
            swresample
            swscale
            x264
    )

    target_compile_options(orientview
            -isystem
            /usr/local/include
    )
endif()

install(TARGETS orientview
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_BINDIR})
add_custom_command(TARGET orientview POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:orientview>/data)



# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
