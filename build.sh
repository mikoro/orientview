#!/bin/bash
(
  cd "$(dirname "$0")"
  docker run --rm -v $(pwd):/project -ti orientview-compile "qmake && make"
)