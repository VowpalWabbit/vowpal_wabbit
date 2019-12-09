#!/bin/bash
set -e
set -x

cd $1
mkdir -p build
cd build

cmake .. -DBoost_NO_BOOST_CMAKE=ON -DCMAKE_BUILD_TYPE=Release -DWARNINGS=Off -DDO_NOT_BUILD_VW_C_WRAPPER=On -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j ${NUM_PROCESSORS}
