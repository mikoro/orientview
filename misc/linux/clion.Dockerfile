FROM orientview-build:latest

# Set environment variables to disable interactive prompts
ENV DEBIAN_FRONTEND=noninteractive    

RUN apt-get update && apt-get install -y \
    cmake \
    gdb \
    # To connect to X11 server
    libx11-6 \
    libx11-xcb1 \
    libxkbcommon-x11-0 \
    libxcb1 \
    libxrender1 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Cleanup
RUN apt-get remove --purge -y git && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the default command to run the development build script
ENTRYPOINT ["/bin/bash"]
