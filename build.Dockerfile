FROM ubuntu:24.04

# Set environment variables to disable interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    # To clone L-SMASH
    git \
    # FFmpeg
    ffmpeg \
    libavcodec-dev \
    libavdevice-dev \
    libavfilter-dev \
    libavformat-dev \
    libavutil-dev \
    libswscale-dev \
    libx264-dev \
    # OpenCV
    libopencv-contrib-dev \
    # Qt
    libqt5svg5-dev \
    qt5-qmake \
    qtbase5-dev \
    qtdeclarative5-dev

RUN git clone --depth 1 https://github.com/l-smash/l-smash.git l-smash && \
    cd l-smash && \
    make install 

WORKDIR /project

# Cleanup
RUN apt-get remove --purge -y git && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /l-smash

# Set the default command to run the development build script
ENTRYPOINT ["/bin/bash", "-c"]
