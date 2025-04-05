cmake_minimum_required(VERSION 3.16)

project(lsmash VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

set(SOURCES
  common/alloc.c
  common/bits.c
  common/bytes.c
  common/list.c
  common/multibuf.c
  common/osdep.c
  common/utils.c

  codecs/a52.c
  codecs/alac.c
  codecs/description.c
  codecs/dts.c
  codecs/h264.c
  codecs/hevc.c
  codecs/id.c
  codecs/mp4sys.c
  codecs/mp4a.c
  codecs/mp4v.c
  codecs/nalu.c
  codecs/qt_wfex.c
  codecs/vc1.c
  codecs/wma.c

  importer/a52_imp.c
  importer/adts_imp.c
  importer/als_imp.c
  importer/amr_imp.c
  importer/dts_imp.c
  importer/importer.c
  importer/isobm_imp.c
  importer/mp3_imp.c
  importer/nalu_imp.c
  importer/vc1_imp.c
  importer/wave_imp.c

  core/box.c
  core/box_default.c
  core/box_type.c
  core/chapter.c
  core/file.c
  core/fragment.c
  core/isom.c
  core/meta.c
  core/print.c
  core/read.c
  core/summary.c
  core/timeline.c
  core/write.c
)

add_library(lsmash STATIC ${SOURCES})

target_include_directories(lsmash
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS lsmash
    EXPORT lsmashConfig
    ARCHIVE DESTINATION lib
)

install(EXPORT lsmashConfig
    DESTINATION share/lsmash
    NAMESPACE lsmash::
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/lsmash.h
    DESTINATION include
)