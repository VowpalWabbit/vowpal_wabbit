cmake_minimum_required (VERSION 3.5)
project (ds_python)

set (CMAKE_CXX_STANDARD 14)

find_package(PythonLibs 3 REQUIRED)
message("Python libs: ${PYTHONLIBS_VERSION_STRING}")

find_package(PythonInterp 3 REQUIRED)

find_package(SWIG REQUIRED)
include(UseSWIG)

include_directories(${Boost_INCLUDE_DIRS} ${cpprest_INCLUDE_DIRS} ${vw_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}) 
include_directories(${vw_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS}) 

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "swig")

set_property(SOURCE ds_api_python.i ../ds_api.i PROPERTY CPLUSPLUS ON)
SET(CMAKE_SWIG_OUTDIR "swig")

SWIG_ADD_MODULE(decision_service python ds_api_python.i)
SWIG_LINK_LIBRARIES(decision_service ds ${PYTHON_LIBRARIES})

# chicken and egg dependency. we can't disocver unit tests until we build it
# discover_python_unittest("test")

# FindOpenSSL doesn't provide openssl binary, otherwise we could generate on the fly
# openssl req -new -x509 -keyout unittest.pem -out unittest.pem -days 536500 -nodes -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com"

# TODO: just grep the file using discover_python_unittest 
add_test(test.test_ds_api.TestDecisionServiceConfiguration.test_not_found ${PYTHON_EXECUTABLE} "-m" "unittest" "-v" test.test_ds_api.TestDecisionServiceConfiguration.test_not_found)
add_test(test.test_ds_api.TestDecisionServiceConfiguration.test_download ${PYTHON_EXECUTABLE} "-m" "unittest" "-v" test.test_ds_api.TestDecisionServiceConfiguration.test_download)
add_test(test.test_ds_api.TestDecisionServiceClient.test_rank ${PYTHON_EXECUTABLE} "-m" "unittest" "-v" test.test_ds_api.TestDecisionServiceClient.test_rank)
