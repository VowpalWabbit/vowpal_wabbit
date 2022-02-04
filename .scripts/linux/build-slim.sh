#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
cmake -S ./vowpalwabbit/slim -B build -G Ninja -DBUILD_TESTING=On
cmake --build build --target vwslim vw-slim-test
