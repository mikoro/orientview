# Building on Linux

## With Docker

1. Install [Docker Engine](https://docs.docker.com/engine/install/)
2. Cd into the project root dir
3. Build the building docker image: `docker build -t compile -f build.Dockerfile .`
4. Run `build.sh`

## Without Docker

Look at the [build.Dockerfile](../../build.Dockerfile) and do the same installation steps.