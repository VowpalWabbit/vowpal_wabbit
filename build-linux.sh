#!/bin/bash
set -e

export PATH="$HOME/miniconda/bin:$PATH"

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
