#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Run unit tests
cd $REPO_DIR/build
ctest --verbose --output-on-failure --label-regex VWTestList --parallel 2

# Run integration tests
cd $REPO_DIR
python3 test/run_tests.py --fuzzy_compare --exit_first_fail --epsilon 0.001 --ignore_dirty --include_flatbuffers
python3 test/run_tests.py --fuzzy_compare --exit_first_fail --epsilon 0.001 --ignore_dirty --extra_options=--onethread --include_flatbuffers
python3 test/run_tests.py --ignore_dirty --test_spec test/slow.vwtest.json --timeout 180
