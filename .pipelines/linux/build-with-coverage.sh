#!/bin/bash
set -e
set -x

# Clear out build directory then build using GCov and run one set of tests again
cd $1
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DGCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On
make vw-bin -j ${NUM_PROCESSORS}
