#!/bin/sh
make all
make python
make test
cd test
./test_race_condition.sh
cd ..
make test_gcov --always-make
cd python
source activate test-python27
pip install pytest readme_renderer
python setup.py check -mrs
python setup.py install
py.test tests
source deactivate
cd ..
