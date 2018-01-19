cmake_minimum_required (VERSION 3.5)
project (ds_python)

set (CMAKE_CXX_STANDARD 14)

# ****** WARNING ******
# Every single line below was modified at least 3 times until I arrived at the structure below
# as the python version and especially the include dirs leak between python 2 and 3
# ****** WARNING ******

include_directories(${Boost_INCLUDE_DIRS} ${cpprest_INCLUDE_DIRS} ${vw_INCLUDE_DIRS} ${ds_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}) 
set_property(SOURCE ds_api_python.i ../ds_api.i PROPERTY CPLUSPLUS ON)

# need macro for unset to work (as function's scope doesn't work here)
macro (ds_pythonbuild PYVERSION)
    find_package(PythonLibs ${PYVERSION} EXACT)
    find_package(PythonInterp ${PYVERSION} EXACT)

    if (PYTHON_INCLUDE_DIRS)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "swig${PYVERSION}")
        set(CMAKE_SWIG_OUTDIR "swig${PYVERSION}")
        set(PYVERSION ${PYVERSION})
        set(ds_api_python "swig${PYVERSION}/ds_api_python.i")

        configure_file(../ds_api_python.i.in ${ds_api_python})
        set_property(SOURCE ${ds_api_python} PROPERTY CPLUSPLUS ON)

        set_source_files_properties("swig${PYVERSION}/ds_api_pythonPYTHON_wrap"
                    PROPERTIES COMPILE_FLAGS "-I${PYTHON_INCLUDE_DIRS} -DSWIG_PYTHON")

        SWIG_ADD_MODULE("decision_service${PYVERSION}" python ${ds_api_python})
        SWIG_LINK_LIBRARIES("decision_service${PYVERSION}" ds ${PYTHON_LIBRARIES})

        ADD_CUSTOM_COMMAND(TARGET "_decision_service${PYVERSION}"
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy "swig${PYVERSION}/decision_service${PYVERSION}.py" "swig${PYVERSION}/decision_service.py")

          # chicken and egg dependency. we can't disocver unit tests until we build it
        # discover_python_unittest("test")

        # FindOpenSSL doesn't provide openssl binary, otherwise we could generate on the fly
        # openssl req -new -x509 -keyout unittest.pem -out unittest.pem -days 536500 -nodes -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com"

        if (${PYVERSION} STREQUAL 3)
            # the http server is only included with python3

            # TODO: just grep the file using discover_python_unittest 
            add_test(test.test_ds_api.TestDecisionServiceConfiguration.test_not_found ${PYTHON_EXECUTABLE} "-m" "unittest" "-v" test.test_ds_api.TestDecisionServiceConfiguration.test_not_found)
            add_test(test.test_ds_api.TestDecisionServiceConfiguration.test_download ${PYTHON_EXECUTABLE} "-m" "unittest" "-v" test.test_ds_api.TestDecisionServiceConfiguration.test_download)
            add_test(test.test_ds_api.TestDecisionServiceClient.test_rank ${PYTHON_EXECUTABLE} "-m" "unittest" "-v" test.test_ds_api.TestDecisionServiceClient.test_rank)
        endif()
    endif()

    # yeah, scoping of functions is not really working: https://github.com/pybind/pybind11/issues/748
    unset(PYTHONINTERP_FOUND CACHE)
    unset(PYTHON_EXECUTABLE CACHE)
    unset(PYTHON_VERSION_STRING CACHE)
    unset(PYTHON_VERSION_MAJOR CACHE)
    unset(PYTHON_VERSION_MINOR CACHE)
    unset(PYTHON_VERSION_PATCH CACHE)
    
    unset(PYTHON_INCLUDE_DIR CACHE)
    unset(PYTHON_INCLUDE_DIRS CACHE)
    unset(PYTHON_LIBRARY CACHE)
    unset(PYTHON_LIBRARIES CACHE)
    unset(PYTHON_MODULE_PREFIX CACHE)
    unset(PYTHON_MODULE_EXTENSION CACHE)
    
    unset(PYTHON_LIBRARY CACHE)
endmacro()

# build python 2 & 3 module
add_subdirectory(python2)
add_subdirectory(python3)