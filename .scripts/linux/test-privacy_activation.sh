#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd test
python3 run_tests.py -f --skip_spanning_tree_tests --test_spec privacy_activation.vwtest.json

cd ../build
./test/unit_test/vw-unit-test.out --run_test=test_feature_is_activated*,test_feature_not_activated*,test_feature_could_be_activated_but_feature_not_initialized*