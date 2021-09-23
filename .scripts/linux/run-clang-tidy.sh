#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$(realpath $SCRIPT_DIR/../../)
cd $REPO_DIR

CHECKS="-*,performance-unnecessary-copy-initialization,performance-for-range-copy,performance-move-const-arg"
cmake -S . -B build -G Ninja "-DCMAKE_CXX_CLANG_TIDY=clang-tidy;-checks=$CHECKS;-warnings-as-errors=$CHECKS;-header-filter=$REPO_DIR/vowpalwabbit/*"
cmake --build build --target vw-bin
