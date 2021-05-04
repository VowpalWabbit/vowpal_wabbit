#!/bin/bash
set -e
set -x

# cd to sources dir
cd $1

cd test
NUM_PROCESSORS=$(cat ../build/nprocs.txt)
python3 run_tests.py -f -j ${NUM_PROCESSORS}

cd ../build
make test_with_output