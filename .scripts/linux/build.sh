#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# If parameter 1 is not supplied, it defaults to Release
BUILD_CONFIGURATION=${1:-Release}

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_CONFIGURATION} -DWARNINGS=Off -DWARNING_AS_ERROR=On -DVW_BUILD_VW_C_WRAPPER=Off -DBUILD_JAVA=On -DBUILD_PYTHON=Off -DBUILD_TESTING=On -DBUILD_EXPERIMENTAL_BINDING=On -DVW_FEAT_FLATBUFFERS=On -DVW_FEAT_CSV=On -DVW_FEAT_CB_GRAPH_FEEDBACK=On
cmake --build build --target all
