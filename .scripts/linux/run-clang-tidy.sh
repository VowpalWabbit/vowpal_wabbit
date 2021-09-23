#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$(realpath $SCRIPT_DIR/../../)
cd $REPO_DIR

CONFIG=$(cat $REPO_DIR/.scripts/linux/clang-tidy-ci-config.yml | tr -d "\n")
cmake -S . -B build -G Ninja -D "CMAKE_CXX_CLANG_TIDY=clang-tidy;-config=$CONFIG;-header-filter=$REPO_DIR/vowpalwabbit/*"
cmake --build build --target vw-bin
