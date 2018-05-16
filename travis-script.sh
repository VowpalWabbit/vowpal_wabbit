#!/bin/sh
cd /vw
make all
if [[ "$LEGACY" == "false" ]]; then make vwslim ; fi
make python
mvn clean test -f java/pom.xml
make test
if [[ "$LEGACY" == "false" ]]; then make vwslimtests ; fi
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
