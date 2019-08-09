#!/bin/bash
set -e
set -x

# Miniconda is installed to home in docker file
export PATH="/usr/local/miniconda/bin:$PATH"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Clear build dir as it uses the conda deps.
rm -rf build

# Run python build and tests
source activate test-python36
python setup.py check -mrs
python setup.py install --user
py.test ./python/tests/
source deactivate
