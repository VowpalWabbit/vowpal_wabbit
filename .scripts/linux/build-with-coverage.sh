#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTING=On -DBUILD_FLATBUFFERS=On -DCMAKE_EXE_LINKER_FLAGS="-lgcov" -DCMAKE_CXX_FLAGS="-fprofile-arcs -ftest-coverage -fno-strict-aliasing -pg"
cmake --build build
