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
cmake .. -DBUILD_BENCHMARKS=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off
NUM_PROCESSORS=$(cat nprocs.txt)
make vw-benchmarks.out -j ${NUM_PROCESSORS}
