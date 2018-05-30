#!/bin/sh
# use miniconda for python package testing
wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh;
bash miniconda.sh -b -p $HOME/miniconda
export PATH="$HOME/miniconda/bin:$PATH"
hash -r
conda config --set always_yes yes --set changeps1 no
conda update -q conda
conda create -q -n test-python27 python=2.7 nomkl numpy scipy scikit-learn

make all
make python
make test
cd test
./test_race_condition.sh
cd ..
make test_gcov --always-make
cd python
source activate test-python27
sudo pip install pytest readme_renderer
python setup.py check -mrs
python setup.py install
py.test tests
source deactivate
cd ..
