#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On -DBUILD_FLATBUFFERS=On
cmake --build build --target vw-bin spanning_tree vw-unit-test.out
