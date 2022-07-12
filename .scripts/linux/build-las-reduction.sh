#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DWARNINGS=OFF -DVW_BUILD_LARGE_ACTION_SPACE=On -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTING=On -DBUILD_FLATBUFFERS=Off
cmake --build build --target vw_cli_bin vw-unit-test.out
