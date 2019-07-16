#!/bin/bash
set -e
set -x

cd $1
mkdir -p build
cd build

NUM_PROCESSORS=$(cat nprocs.txt)
make test_with_output -j ${NUM_PROCESSORS}
