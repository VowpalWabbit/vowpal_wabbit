#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
cmake -S . -B build -G Ninja -DBUILD_BENCHMARKS=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_FLATBUFFERS=On -DBUILD_CSV=On
cmake --build build --target vw-benchmarks.out
