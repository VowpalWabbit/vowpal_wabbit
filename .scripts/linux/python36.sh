#!/bin/bash
set -e
set -x

# Miniconda is installed to root in docker file
export PATH="/opt/miniconda/bin:$PATH"

# Clear build dir as it uses the conda deps.
cd $1
rm -fr build
mkdir build
cd build

# Run python build and tests
source activate test-python36
python setup.py check -mrs
python setup.py install
py.test ./python/tests/
source deactivate
