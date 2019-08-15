#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Clear build dir as it uses the conda deps.
rm -rf build

# Run python build and tests
conda activate test-python36
python setup.py check -mrs
python setup.py install --user
py.test ./python/tests/
conda deactivate
