#!/bin/bash
set -e
set -x

cd $1
mkdir -p build
cd build

# NUM_PROCESSORS=$(cat nprocs.txt)
make help
make vw-unit-test.out
test/unit_test/vw-unit-test.out -l all MacCI
make test_with_output -j ${NUM_PROCESSORS} -- MacCI