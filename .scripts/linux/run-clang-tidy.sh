#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cmake --version

clang --version

cmake -S . -B build -G Ninja -DFMT_SYS_DEP=On -DSPDLOG_SYS_DEP=On -DVW_BOOST_MATH_SYS_DEP=On -DBUILD_TESTING=Off -DDO_NOT_BUILD_VW_C_WRAPPER=On
run-clang-tidy -p build -header-filter=vw/*
