#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd build
export BOOST_TEST_LOG_LEVEL=unit_scope
ctest --verbose --output-on-failure --overwrite MemoryCheckCommandOptions="--leak-check=full --error-exitcode=100" --label-regex VWTestList --timeout 2400 -T memcheck
