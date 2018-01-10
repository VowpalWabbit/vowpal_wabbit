# Boost_USE_STATIC_LIBS ... there is no program options runtime package available, linking statically is the best we can do
cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -DBoost_USE_STATIC_LIBS=ON .
make -j

cd decision_service/python

# TODO: move to CMake to be able to call both Python 2 and 3
python setup.py install
