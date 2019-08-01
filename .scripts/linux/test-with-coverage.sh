#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

cd test
export PATH=../build/vowpalwabbit/:$PATH && ./RunTests -d -fe -E 0.001
