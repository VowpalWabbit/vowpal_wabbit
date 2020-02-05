#!/bin/bash
set -e
set -x

export PATH="$HOME/miniconda/bin:$PATH"

# Check if any clang-formatting necessary

cd /vw
./utl/clang-format check

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

cd /vw
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DWARNINGS=Off -DDO_NOT_BUILD_VW_C_WRAPPER=On -DBUILD_JAVA=On -DBUILD_PYTHON=On -DBUILD_TESTS=On
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j ${NUM_PROCESSORS}
make test_with_output
cd ..

# Run Java build and test
mvn verify -f java/pom.xml

# Run python build and tests
source activate test-python27
pip install pytest readme_renderer pandas
python setup.py check -mrs
python setup.py install
py.test python/tests
source deactivate

# Clear out build directory then build using GCov and run one set of tests again
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DGCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On -DDO_NOT_BUILD_VW_C_WRAPPER=On
make vw-bin vw-unit-test.out spanning_tree -j ${NUM_PROCESSORS}
./test/unit_test/vw-unit-test.out
cd ../test
./RunTests -d -fe -E 0.001
cd ..
