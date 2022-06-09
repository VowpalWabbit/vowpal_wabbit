#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

mkdir -p build
cd build
# Boost unit tests don't like the static linking
# /usr/local/bin/gcc + g++ is 9.2.0 version
cmake .. -DCMAKE_BUILD_TYPE=Release -DWARNINGS=Off -DBUILD_JAVA=On -DBUILD_DOCS=Off -DBUILD_FLATBUFFERS=On\
 -DBUILD_PYTHON=Off -DCMAKE_C_COMPILER=/usr/local/bin/gcc -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ \
 -DBUILD_TESTING=Off
NUM_PROCESSORS=$(nproc)
make all -j ${NUM_PROCESSORS}
