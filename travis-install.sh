#!/bin/sh
if [[ "$LEGACY" == "false" ]]; then SUDO=sudo ; fi
$SUDO apt-get update -qq
$SUDO apt-get install -y -qq libboost-all-dev
$SUDO apt-get install -y maven
if [[ "$LEGACY" == "false" ]]; then 
sudo apt-get install -y libgtest-dev google-mock wget pip conda
fi
pip install cpp-coveralls wheel
# use miniconda for python package testing
wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh;
bash miniconda.sh -b -p $HOME/miniconda
export PATH="$HOME/miniconda/bin:$PATH"
hash -r
conda config --set always_yes yes --set changeps1 no
conda update -q conda
conda create -q -n test-python27 python=2.7 nomkl numpy scipy scikit-learn

