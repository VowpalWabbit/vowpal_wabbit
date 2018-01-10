cmake -DCMAKE_INSTALL_PREFIX=${PREFIX}
make -j

cd decision_service/python

# TODO: move to CMake to be able to call both Python 2 and 3
python setup.py install
