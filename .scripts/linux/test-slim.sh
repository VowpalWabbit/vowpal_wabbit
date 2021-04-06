#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd ./build/vowpalwabbit/slim/test/
# Needs to be run from this directory as tests load files relative to current dir.
./vw-slim-test
