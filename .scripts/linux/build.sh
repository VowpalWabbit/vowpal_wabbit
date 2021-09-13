#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# If parameter 1 is not supplied, it defaults to Release
BUILD_CONFIGURATION=${1:-Release}

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_CONFIGURATION} -DWARNINGS=Off -DWARNING_AS_ERROR=On -DDO_NOT_BUILD_VW_C_WRAPPER=On -DBUILD_JAVA=On -DBUILD_PYTHON=Off -DBUILD_TESTS=On -DBUILD_EXPERIMENTAL_BINDING=On -DBUILD_FLATBUFFERS=On
cmake --build build --target all
