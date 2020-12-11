#!/bin/bash
set -e
set -x

# cd to sources dir
cd $1

cd test
python3 run_tests.py -f -j $(nproc)

cd ../build
make test_with_output