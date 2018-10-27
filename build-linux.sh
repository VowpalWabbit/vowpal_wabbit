#!/bin/bash
set -e
set -x

export PATH="$HOME/miniconda/bin:$PATH"

sudo apt remove --yes --force-yes cmake

# Upgrade CMake
version=3.5
build=2
mkdir ~/temp
cd ~/temp
wget https://cmake.org/files/v$version/cmake-$version.$build-Linux-x86_64.sh
sudo mkdir /opt/cmake
sudo sh cmake-$version.$build-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
sudo ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DWARNINGS=OFF
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j ${NUM_PROCESSORS}
make test_with_output
cd ..

# Run Java build and test
mvn clean test -f java/pom.xml

# Run python build and tests
cd python
source activate test-python27
pip install pytest readme_renderer pandas
python setup.py check -mrs
python setup.py install
py.test tests
source deactivate
cd ..

# Clear out build directory then build using GCov and run one set of tests again
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DGCOV=ON -DWARNINGS=OFF
make vw-bin -j ${NUM_PROCESSORS}
ctest -R RunTests_pass_2
