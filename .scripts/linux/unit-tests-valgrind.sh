#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd build
valgrind --quiet --error-exitcode=100 --track-origins=yes --leak-check=full ./test/unit_test/vw-unit-test.out
valgrind --quiet --error-exitcode=100 --track-origins=yes --leak-check=full ./bindings/c/test/vw_c_api_unit_test
