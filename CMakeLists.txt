# cmake -S . -B build
# cmake --build build

cmake_minimum_required(VERSION 3.16)

if (WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

execute_process(
  COMMAND git describe --tags --abbrev=0
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  RESULT_VARIABLE GIT_RESULT
  OUTPUT_VARIABLE GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX REPLACE "^v" "" PROJECT_VERSION_STRING "${GIT_TAG}")

if(NOT PROJECT_VERSION_STRING MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(WARNING "Could not determine current version from git tags, using fallback version 0.0.0")
  set(PROJECT_VERSION_STRING "0.0.0")
endif()

if (WIN32 OR APPLE)
  set(EXE_NAME OrientView)
else()
  set(EXE_NAME orientview)
endif()

project(${EXE_NAME} VERSION ${PROJECT_VERSION_STRING} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(GNUInstallDirs)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O1 /Gy")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O1 /Gy")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /INCREMENTAL:NO")
  elseif(APPLE)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -ffunction-sections -fdata-sections")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-dead_strip")
  else()
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -ffunction-sections -fdata-sections")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -s")
  endif()
endif()

find_package(FFMPEG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

file(GLOB_RECURSE SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

if(WIN32)
  list(APPEND SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/orientview.rc")
endif()

add_executable(${EXE_NAME} ${SRC_FILES})

target_include_directories(${EXE_NAME} PRIVATE
  ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(${EXE_NAME} PRIVATE
  ${FFMPEG_LIBRARY_DIRS}
)

target_link_libraries(${EXE_NAME} PRIVATE
  ${FFMPEG_LIBRARIES}
  fmt::fmt
  glm::glm
  imgui::imgui
  nlohmann_json::nlohmann_json
  pugixml::pugixml
  SDL3::SDL3
  spdlog::spdlog
)

add_custom_command(TARGET ${EXE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${EXE_NAME}>/data
)

if(WIN32)
  set(PLATFORM_NAME "windows")
  set(DATA_DIR_PATH "data")

  set_target_properties(${EXE_NAME} PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
  )

  install(TARGETS ${EXE_NAME} RUNTIME DESTINATION .)
  install(DIRECTORY data DESTINATION .)

elseif(APPLE)
  set(PLATFORM_NAME "macos")
  set(DATA_DIR_PATH "data")

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

else()
  set(PLATFORM_NAME "linux")
  set(DATA_DIR_PATH "${CMAKE_INSTALL_FULL_DATADIR}/orientview/data")

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

target_compile_definitions(${EXE_NAME} PRIVATE
  ORIENTVIEW_DATA_DIR_PATH="${DATA_DIR_PATH}"
  ORIENTVIEW_VERSION="${PROJECT_VERSION_STRING}"
)

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
