#!/bin/bash
set -e
set -x

# Miniconda is installed to root in docker file
export PATH="/opt/miniconda/bin:$PATH"

# Run python build and tests
cd $1
source activate test-python27
sudo pip install pytest readme_renderer pandas
python setup.py check -mrs
sudo python setup.py install
py.test tests
source deactivate
