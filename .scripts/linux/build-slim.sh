#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
cmake -S . -B build -G Ninja \
    -DBUILD_TESTING=On \
    -DVW_FEAT_FLATBUFFERS=Off \
    -DRAPIDJSON_SYS_DEP=Off \
    -DFMT_SYS_DEP=Off \
    -DSPDLOG_SYS_DEP=Off \
    -DVW_ZLIB_SYS_DEP=Off \
    -DVW_BOOST_MATH_SYS_DEP=Off

cmake --build build --target vw_slim vw_slim_test
