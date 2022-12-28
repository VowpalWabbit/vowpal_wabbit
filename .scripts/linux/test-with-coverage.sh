#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd $REPO_DIR/build
ctest --verbose --output-on-failure --label-regex VWTestList --parallel 2

cd $REPO_DIR/test
python3 run_tests.py -f -j $(nproc) --include_flatbuffers --clean_dirty -E 0.001
