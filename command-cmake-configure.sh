#!/bin/bash
rm -r build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=/home/somayaji/s/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_FLATBUFFERS=ON