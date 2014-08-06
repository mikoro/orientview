# Compiling external libraries

## Prerequisites

### Environment 1

* Install Intel C++ Composer XE [https://software.intel.com/en-us/intel-composer-xe](https://software.intel.com/en-us/intel-composer-xe)
* Install MinGW using the installer to `C:\MinGW` (mingw32-base + msys-base + msys-coreutils-ext) [http://www.mingw.org/wiki/MinGW](http://www.mingw.org/wiki/MinGW)
* Download YASM (yasm-1.2.0-win64.exe -> msys/bin/yasm.exe) [http://yasm.tortall.net/Download.html](http://yasm.tortall.net/Download.html)
* Open CMD and run: `"C:\Program Files (x86)\Intel\Composer XE 2013 SP1\bin\compilervars.bat" intel64`
* With that CMD, run: `C:\MinGW\msys\1.0\msys.bat`
* Check that `which link` -> /c/Program Files (x86)/Microsoft Visual Studio 12.0/VC/BIN/amd64/link.exe (not /bin/link.exe, if so, rename the latter to something else)
* `export CC=icl`

### Environment 2

* Download latest MinGW-w64 (64-bit/threads-posix/seh): `http://sourceforge.net/projects/mingwbuilds/files/host-windows/releases/`
* Extract to `C:\MinGW64`
* Download MSYS [http://downloads.sourceforge.net/mingw/MSYS-1.0.11.exe](http://downloads.sourceforge.net/mingw/MSYS-1.0.11.exe)
* Install to `C:\MinGW64\msys\1.0` and and give `c:/MinGW64` as the MinGW path
* Open CMD and run: `C:\MinGW64\msys\1.0\msys.bat`

## FFmpeg

* Environment 1
* `git clone git://source.ffmpeg.org/ffmpeg.git`
* `./configure --toolchain=icl --prefix=./install --enable-gpl --disable-static --enable-shared --disable-programs --disable-doc`
* `make install`

## x264

* Environment 1
* `git clone git://git.videolan.org/x264.git`
* `./configure --prefix=./install --disable-cli --enable-shared --enable-win32thread --disable-interlaced --bit-depth=8 --chroma-format=420`
* `make install`

## L-SMASH

* Environment 2
* `git clone https://github.com/l-smash/l-smash.git`
* `./configure --prefix=./install --disable-static --enable-shared --extra-cflags="-m64" --extra-ldflags="-m64 -static-libgcc -static-libstdc++"`
* `make`
* Run the link command again (gcc -shared...) and modify the *Wl* flag to following: `-Wl,--output-def,liblsmash.def,--out-implib,liblsmash.a`
* Open CMD in the l-smash directory and run: `"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64`
* `lib /machine:x64 /def:liblsmash.def`
* `make install`

## OpenCV

* `git clone https://github.com/Itseez/opencv.git`
* Open CMake GUI, select opencv directory, set binary directory to opencv/build
* Configure -> yes -> vs 2013 win64 -> native
* Change (at least): no docs, no tests, no debug info, no static crt, no perf tests, no openexr
* Generate
* Open build/OpenCV.sln -> Solution Use Intel C++ -> Release x64
* ALL_BUILD -> Build
* INSTALL -> Build
