#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd test
source activate test-python36
python3 run_tests.py -f -j $(nproc) --include_flatbuffers
source deactivate

cd ../build
make test_with_output

