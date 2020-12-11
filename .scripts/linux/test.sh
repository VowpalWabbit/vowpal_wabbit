#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# cd build
# make test_with_output

python --version
python3 --version

python3 -m pip install future-fstrings

cd test
python3 run_tests.py -f -j $(nproc)
