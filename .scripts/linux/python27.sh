#!/bin/bash
set -e
set -x

# Miniconda is installed to home in docker file
export PATH="/usr/local/miniconda/bin:$PATH"

# Clear build dir as it uses the conda deps.
cd $1
rm -rf build

# Run python build and tests
cd $1
source activate test-python27
python setup.py check -mrs
python setup.py install --user
py.test ./python/tests/
source deactivate
