#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Check out the library.
git clone https://github.com/google/benchmark.git
cd benchmark
git checkout v1.6.1

# Generate build system files with cmake.
cmake -S . -B build -G Ninja -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DBENCHMARK_ENABLE_TESTING=Off -DCMAKE_BUILD_TYPE=Release
# Install globally
sudo cmake --build "build" --config Release --target install
