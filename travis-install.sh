#!/bin/bash
if [[ "$LEGACY" == "true" ]]; then 
sudo apt-get update -qq
sudo apt-get install -y -qq libboost-all-dev
sudo apt-get install -y maven
sudo pip install cpp-coveralls wheel
else
add-apt-repository ppa:webupd8team/java
apt-get update -qq
apt-get install -y -qq libboost-all-dev
apt-get install -y python-setuptools python-dev build-essential
apt-get install -y maven libgtest-dev google-mock wget zlib1g-dev
apt-get install -y oracle-java8-installer
pip install cpp-coveralls wheel
fi
# use miniconda for python package testing
wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh;
bash miniconda.sh -b -p $HOME/miniconda
export PATH="$HOME/miniconda/bin:$PATH"
hash -r
conda config --set always_yes yes --set changeps1 no
conda update -q conda
conda create -q -n test-python27 python=2.7 nomkl numpy scipy scikit-learn

