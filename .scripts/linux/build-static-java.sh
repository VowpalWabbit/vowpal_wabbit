#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

mkdir -p build
cd build
# Boost unit tests don't like the static linking
cmake .. -DCMAKE_BUILD_TYPE=Release -DWARNINGS=Off -DBUILD_JAVA=On -DBUILD_DOCS=Off \
 -DBUILD_PYTHON=Off -DSTATIC_LINK_VW_JAVA=On -DCMAKE_C_COMPILER=/usr/local/bin/gcc -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ \
 -DBUILD_TESTS=Off
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j ${NUM_PROCESSORS}
