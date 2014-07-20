# Compiling external libraries

## Prerequisites

* Install Intel C++ Composer XE [https://software.intel.com/en-us/intel-composer-xe](https://software.intel.com/en-us/intel-composer-xe)
* Install MinGW (mingw32-base + msys-base + msys-coreutils-ext) [http://www.mingw.org/wiki/MinGW](http://www.mingw.org/wiki/MinGW)
* Install YASM (yasm-1.2.0-win64.exe -> msys/bin/yasm.exe) [http://yasm.tortall.net/Download.html](http://yasm.tortall.net/Download.html)
* Open CMD and run: `"C:\Program Files (x86)\Intel\Composer XE 2013 SP1\bin\compilervars.bat" intel64`
* With that CMD, run: `C:\MinGW\msys\1.0\msys.bat`
* Check that `which link` -> /c/Program Files (x86)/Microsoft Visual Studio 12.0/VC/BIN/amd64/link.exe (not /bin/link.exe, if so, rename the latter to something else)
* `export CC=icl`

## FFmpeg

* `git clone git://source.ffmpeg.org/ffmpeg.git`
* `./configure --toolchain=icl --prefix=./install --enable-gpl --disable-static --enable-shared --disable-programs --disable-doc`
* `make install`

## OpenCV

* `git clone https://github.com/Itseez/opencv.git`
* Open cmake gui, select opencv directory, set binary directory to opencv/build
* Configure -> yes -> vs 2013 win64 -> native
* Deselect what is not needed (no image libs, no docs, no tests, no static crt, no with *)
* Generate
* Open build/OpenCV.sln -> solution use intel c++ -> Release x64
* ALL_BUILD -> Build
* INSTALL -> Build

## GPAC

* `svn co svn://svn.code.sf.net/p/gpac/code/trunk/gpac`
* Open build\msvc11\gpac.sln -> convert -> solution use intel c++ -> Release x64
* Build libgpac project

## x264

* `git clone git://git.videolan.org/x264.git` (same root directory as gpac)
* Modify the *configure* file to always pass the gpac test (line ~966) 
* `./configure --prefix=./install --disable-cli --enable-shared --extra-ldflags="-L../gpac/bin/x64/Release" --extra-cflags="-I../gpac/include"`
* `make install`
