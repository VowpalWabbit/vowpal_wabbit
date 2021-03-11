#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Clear out build directory then build using GCov and run one set of tests again
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DGCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On -DBUILD_FLATBUFFERS=On
NUM_PROCESSORS=$(cat nprocs.txt)
make vw-bin spanning_tree vw-unit-test.out -j ${NUM_PROCESSORS}
