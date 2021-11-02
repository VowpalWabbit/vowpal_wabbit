#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd test
python3 run_tests.py -f -j $(nproc) --include_flatbuffers --clean_dirty -E 0.001
../build/test/unit_test/vw-unit-test.out
