#!/bin/bash

set -e -u

CMAKE_DIR=$(dirname $0)
BUILD_DIR="$CMAKE_DIR/build"

cmake -H${CMAKE_DIR} -B${BUILD_DIR}
cmake --build ${BUILD_DIR} -- -j3
