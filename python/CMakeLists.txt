if(NOT DEFINED PY_VERSION)
  set(PY_VERSION 2.7)
endif()
string(REPLACE . "" PY_VERSION_STRIPPED ${PY_VERSION})

message("Building Python module for Python version ${PY_VERSION}")
find_package(PythonLibs ${PY_VERSION} REQUIRED)
find_package(Boost REQUIRED COMPONENTS system python${PY_VERSION_STRIPPED})

# Use position independent code for all targets in this directory
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT WIN32)
  set(EXPORT_DYNAMIC -Wl,--export-dynamic)
endif()

# Note: Python bindings previously built under c++0x

# create the lib
add_library(pylibvw SHARED pylibvw.cc)
set_target_properties(pylibvw PROPERTIES PREFIX "")
target_link_libraries(pylibvw PUBLIC Boost::system Boost::python${PY_VERSION_STRIPPED} vw allreduce)
target_include_directories(pylibvw PRIVATE ${PYTHON_INCLUDE_PATH})
target_compile_options(pylibvw PRIVATE ${EXPORT_DYNAMIC})
