#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DWARNINGS=OFF -DBUILD_PRIVACY_ACTIVATION=On -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTING=On -DBUILD_FLATBUFFERS=Off
cmake --build build --target vw-bin vw-unit-test.out
