# The easy way (GitHub Actions)

[Fork the repo](https://github.com/mikoro/orientview/fork) at GitHub, enable Actions for the fork, make the changes, push to GitHub and let Actions build all the platforms for you.

# Build locally

Note that the first CMake configure run will take a long time as vcpkg will build all the dependency libraries from source.

## Windows

1. Install latest [Visual Studio](https://visualstudio.microsoft.com/downloads/). Enable C++ development when installing.
2. Install latest [CMake](https://cmake.org/download/)
3. `git clone --recurse-submodules https://github.com/mikoro/orientview.git`
4. `cd orientview; mkdir build; cd build; cmake ..; cmake --build .`

Alternatively after `cmake ..` one can open the `OrientView.sln` in Visual Studio and build there.

## Linux

1. `sudo apt update && sudo apt install build-essential cmake nasm`
2. `git clone --recurse-submodules https://github.com/mikoro/orientview.git`
3. `cd orientview && mkdir build && cd build && cmake ..`

## Mac

TBD