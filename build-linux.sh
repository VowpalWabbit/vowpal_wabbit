#!/bin/bash
set -e
set -x

export PATH="$HOME/miniconda/bin:$PATH"

# Check if any clang-formatting necessary

cd /vw
./utl/clang-format check

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
cmake .. -DCMAKE_BUILD_TYPE=Release -DGCOV=ON -DWARNINGS=OFF -DBUILD_JAVA=Off -DBUILD_PYTHON=Off -DBUILD_TESTS=On
make vw-bin -j ${NUM_PROCESSORS}
cd ..
cd test
export PATH=../build/vowpalwabbit/:$PATH && ./RunTests -d -fe -E 0.001
cd ..
