#!/bin/bash
set -e
set -x

# Miniconda is installed to root in docker file
export PATH="/opt/miniconda/bin:$PATH"

# Run python build and tests
cd $1
cd python
source activate test-python27
pip install pytest readme_renderer pandas --user
python setup.py check -mrs
python setup.py install
py.test tests
source deactivate
