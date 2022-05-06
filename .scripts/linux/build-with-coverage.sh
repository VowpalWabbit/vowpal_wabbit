#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DVW_GCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTING=On -DBUILD_FLATBUFFERS=On -DBUILD_CSV=On
cmake --build build
