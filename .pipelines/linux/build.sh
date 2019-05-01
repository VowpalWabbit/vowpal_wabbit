#!/bin/bash
set -e
set -x

cd $1
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DWARNINGS=Off -DDO_NOT_BUILD_VW_C_WRAPPER=On -DBUILD_JAVA=On -DBUILD_PYTHON=On -DBUILD_TESTS=On
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j ${NUM_PROCESSORS}
