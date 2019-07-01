#!/bin/bash
set -e
set -x

brew install cmake
brew install boost

cd $1
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DWARNINGS=Off -DDO_NOT_BUILD_VW_C_WRAPPER=On -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j ${NUM_PROCESSORS}

make test_with_output -j ${NUM_PROCESSORS}
