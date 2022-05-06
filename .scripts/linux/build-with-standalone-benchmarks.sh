#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# if cmake doesn't have BUILD_BENCHMARKS add it
if ! grep -q "BUILD_BENCHMARKS" CMakeLists.txt; then
    echo "Adding BUILD_BENCHMARKS option to CMakeLists.txt";
    echo >> CMakeLists.txt
    echo "option(BUILD_ONLY_STANDALONE_BENCHMARKS \"Build only the benchmarks that can run standalone (and do not use vw internals)\" ON)" >> CMakeLists.txt
    echo >> CMakeLists.txt
    echo "add_subdirectory(test/benchmarks)" >> CMakeLists.txt
fi

rm -rf build
cmake -S . -B build -G Ninja -DBUILD_BENCHMARKS=ON -DBUILD_ONLY_STANDALONE_BENCHMARKS=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_FLATBUFFERS=On -DBUILD_CSV=On
cmake --build build --target vw-benchmarks.out
