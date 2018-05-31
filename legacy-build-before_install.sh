#!/bin/bash
sudo apt-get update -qq
sudo apt-get install -qq libboost-all-dev
sudo apt-get install maven
sudo pip install cpp-coveralls wheel

# use miniconda for python package testing
wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh;
bash miniconda.sh -b -p $HOME/miniconda
export PATH="$HOME/miniconda/bin:$PATH"
hash -r
conda config --set always_yes yes --set changeps1 no
conda update -q conda
conda create -q -n test-python27 python=2.7 nomkl numpy scipy scikit-learn
