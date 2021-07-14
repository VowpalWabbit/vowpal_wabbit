#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Check out the library.
git clone https://github.com/google/benchmark.git
# Benchmark requires Google Test as a dependency. Add the source tree as a subdirectory.
git clone https://github.com/google/googletest.git benchmark/googletest
cd benchmark/googletest
git checkout release-1.11.0
# Go to the library root directory
cd ..
# Make a build directory to place the build output.
cmake -E make_directory "build"
# Generate build system files with cmake.
cmake -E chdir "build" cmake -DCMAKE_BUILD_TYPE=Release ../
# Install globally
sudo cmake --build "build" --config Release --target install
