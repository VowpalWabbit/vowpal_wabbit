#!/bin/bash
set -e
set -x

# Miniconda is installed to home in docker file
export PATH="/usr/local/miniconda/bin:$PATH"

# Clear build dir as it uses the conda deps.
cd $1
rm -rf build

# Run python build and tests
source activate test-python36
python setup.py check -mrs
python setup.py install
py.test ./python/tests/
source deactivate
