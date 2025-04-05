cmake_minimum_required(VERSION 3.16)

if (WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg-ports")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" PROJECT_VERSION_STRING)
string(STRIP "${PROJECT_VERSION_STRING}" PROJECT_VERSION_STRING)

if (WIN32 OR APPLE)
  project(OrientView VERSION ${PROJECT_VERSION_STRING} LANGUAGES C CXX)
  set(EXE_NAME OrientView)
else()
  project(orientview VERSION ${PROJECT_VERSION_STRING} LANGUAGES C CXX)
  set(EXE_NAME orientview)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui OpenGL Svg Widgets Xml REQUIRED)
find_package(OpenCV REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(lsmash CONFIG REQUIRED)

pkg_check_modules(X264 REQUIRED IMPORTED_TARGET x264)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
include(GNUInstallDirs)

set(SRC_FILES
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

if(WIN32)
  list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/orientview.rc)
endif()

add_executable(${EXE_NAME} ${SRC_FILES})

qt_add_resources(RESOURCES src/OrientView.qrc)
target_sources(${EXE_NAME} PRIVATE ${RESOURCES})

target_include_directories(${EXE_NAME} PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(${EXE_NAME} PRIVATE
  ${FFMPEG_LIBRARY_DIRS}
)

target_link_libraries(${EXE_NAME} PRIVATE
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

add_custom_command(TARGET ${EXE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${EXE_NAME}>/data
)

if(WIN32)
  set(PLATFORM_NAME "windows")

  set_target_properties(${EXE_NAME} PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
  )

  install(TARGETS ${EXE_NAME} RUNTIME DESTINATION .)
  install(DIRECTORY data DESTINATION .)
elseif(APPLE)
  set(PLATFORM_NAME "macos")

  set_target_properties(${EXE_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platform/macos/Info.plist"
  )

  target_sources(${EXE_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/icons/orientview.icns"
  )

  set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/icons/orientview.icns"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )

  install(TARGETS ${EXE_NAME} BUNDLE DESTINATION .)
  install(DIRECTORY data DESTINATION "${EXE_NAME}.app/Contents/Resources")
else()
  set(PLATFORM_NAME "ubuntu")

  install(
      FILES "platform/linux/orientview.desktop"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
  )

  install(
      FILES "icons/orientview.svg"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps"
  )

  install(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin)
  install(DIRECTORY data DESTINATION share/orientview)
endif()

set(CPACK_PACKAGE_NAME "OrientView")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_STRING}")
set(CPACK_PACKAGE_FILE_NAME "orientview-${CPACK_PACKAGE_VERSION}-${PLATFORM_NAME}")
set(CPACK_PACKAGE_CONTACT "Mikko Ronkainen <firstname@mikkoronkainen.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Orienteering video and map analysis")
set(CPACK_PACKAGE_VENDOR "OrientView")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/mikoro/orientview")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_NSIS_DISPLAY_NAME "OrientView")
set(CPACK_NSIS_PACKAGE_NAME "OrientView")
set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")

set(CPACK_DMG_VOLUME_NAME "OrientView")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Mikko Ronkainen")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")

set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP "Utilities")
set(CPACK_RPM_PACKAGE_VENDOR "OrientView")
set(CPACK_RPM_PACKAGE_URL "https://github.com/mikoro/orientview")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Orienteering video and map analysis")
set(CPACK_RPM_PACKAGE_REQUIRES "")

include(CPack)