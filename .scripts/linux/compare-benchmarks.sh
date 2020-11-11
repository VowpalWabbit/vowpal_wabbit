#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

/usr/local/miniconda/envs/test-python36/bin/python compare.py benchmarks "$1" "$2"
