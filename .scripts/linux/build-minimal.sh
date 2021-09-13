#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# If parameter 1 is not supplied, it defaults to Release
BUILD_CONFIGURATION=${1:-Release}

cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXPERIMENTAL_BINDING=On \
    -DBUILD_FLATBUFFERS=On \
    -DFMT_SYS_DEP=On \
    -DSPDLOG_SYS_DEP=On

cmake --build build --target vw-bin vw-unit-test.out vw_c_api_unit_test spanning_tree to_flatbuff
