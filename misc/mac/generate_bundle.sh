#!/bin/bash

QTBIN=/usr/local/Qt-5.3.1/bin/

rm -rf build OrientView.app Makefile .qmake.stash orientview_plugin_import.cpp

$QTBIN/qmake
make -j4
$QTBIN/macdeployqt OrientView.app -verbose=2

install_name_tool -change lib/libopencv_core.2.4.dylib @executable_path/../Frameworks/libopencv_core.2.4.dylib OrientView.app/Contents/MacOs/OrientView
install_name_tool -change lib/libopencv_imgproc.2.4.dylib @executable_path/../Frameworks/libopencv_imgproc.2.4.dylib OrientView.app/Contents/MacOs/OrientView
install_name_tool -change lib/libopencv_photo.2.4.dylib @executable_path/../Frameworks/libopencv_photo.2.4.dylib OrientView.app/Contents/MacOs/OrientView
install_name_tool -change lib/libopencv_video.2.4.dylib @executable_path/../Frameworks/libopencv_video.2.4.dylib OrientView.app/Contents/MacOs/OrientView
install_name_tool -change lib/libopencv_core.2.4.dylib @executable_path/../Frameworks/libopencv_core.2.4.dylib OrientView.app/Contents/Frameworks/libopencv_imgproc.2.4.dylib
install_name_tool -change lib/libopencv_core.2.4.dylib @executable_path/../Frameworks/libopencv_core.2.4.dylib OrientView.app/Contents/Frameworks/libopencv_photo.2.4.dylib
install_name_tool -change lib/libopencv_imgproc.2.4.dylib @executable_path/../Frameworks/libopencv_imgproc.2.4.dylib OrientView.app/Contents/Frameworks/libopencv_photo.2.4.dylib
install_name_tool -change lib/libopencv_core.2.4.dylib @executable_path/../Frameworks/libopencv_core.2.4.dylib OrientView.app/Contents/Frameworks/libopencv_video.2.4.dylib
install_name_tool -change lib/libopencv_imgproc.2.4.dylib @executable_path/../Frameworks/libopencv_imgproc.2.4.dylib OrientView.app/Contents/Frameworks/libopencv_video.2.4.dylib

rm -rf OrientView.app/Contents/PkgInfo OrientView.app/Contents/Resources/empty.lproj

otool -L OrientView.app/Contents/MacOs/OrientView
