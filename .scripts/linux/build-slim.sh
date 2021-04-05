#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DWARNINGS=OFF -DBUILD_SLIM_VW=On -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On -DBUILD_FLATBUFFERS=Off
NUM_PROCESSORS=$(cat nprocs.txt)
make vwslim vw-slim-test -j ${NUM_PROCESSORS}
