#!/bin/bash

git clone https://github.com/google/flatbuffers.git flatbuffers
cd ./flatbuffers
# 1.10.0 release commit
# git checkout 925c1d77fcc72636924c3c13428a34180c30f96f
mkdir build
cd build
cmake .. -DFLATBUFFERS_BUILD_TESTING=Off -DFLATBUFFERS_INSTALL=On -DCMAKE_BUILD_TYPE=Release -DFLATBUFFERS_BUILD_FLATHASH=Off
make -j `nproc`
echo ~
make DESTDIR=~ install
cd ../../
rm -rf flatbuffers