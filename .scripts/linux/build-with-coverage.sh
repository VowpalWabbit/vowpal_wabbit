#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DVW_GCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTING=On -DVW_FEAT_FLATBUFFERS=On -DVW_FEAT_CSV=On -DVW_FEAT_CB_GRAPH_FEEDBACK=On -DSTD_INV_SQRT=ON
cmake --build build
