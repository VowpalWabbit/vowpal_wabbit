#!/bin/bash

git clone https://github.com/google/googletest.git googletest
cd ./googletest
git checkout v1.10.0
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j `nproc`
make install
cd ../../
rm -rf googletest
