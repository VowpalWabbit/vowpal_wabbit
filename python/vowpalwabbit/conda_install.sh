#!/bin/bash

# check that conda is installed and added to the path
if [ ! $(which conda) ]; then
  echo "No conda found. This installation script is useless in this case. See the standard procedure in the docs." >&2
  exit 1
fi

# a consistent compiler (g++ 7.2.0 at the moment)
conda install --yes gxx_linux-64
# pick up the newly installed compiler
source activate ${CONDA_DEFAULT_ENV}

# boost packages
conda install -c conda-forge --yes boost libboost py-boost zlib cmake

# make a soft link to the compiler, since Makefiles internally use `which g++`
if [ ! -z "${GXX}" ]; then
  ln -sf ${GXX} `dirname ${GXX}`/g++
else
  echo "No compiler linking done" >&2
fi

# set BOOST_XXX variables, that will be used to find boost libs in linking
if [ ! -z "$CONDA_PREFIX" ]; then
  BASE_BOOST=$CONDA_PREFIX
  export BOOST_ROOT=$CONDA_PREFIX
else
  echo '$CONDA_PREFIX is not set - aborting' >&2
  exit 1
fi
export USER_BOOST_INCLUDE="-I $BASE_BOOST/include/boost -I $BASE_BOOST/include"
export USER_BOOST_LIBRARY="-L $BASE_BOOST/lib"

# build the C++ binaries and python package
pip install -e `dirname $0`/../
