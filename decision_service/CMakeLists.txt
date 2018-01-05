cmake_minimum_required (VERSION 3.5)
project (ds)

set (CMAKE_CXX_STANDARD 14)

# BOOST python
find_package(Boost ${Boost_VW_VERSION} COMPONENTS regex)
find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PythonLibs REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${cpprest_INCLUDE_DIRS} ${vw_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}) 
include_directories(${vw_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS}) 

add_library(${PROJECT_NAME} ds_api.cc ds_vw.cc event_hub_client.cc)

target_link_libraries(${PROJECT_NAME} PRIVATE vw ${Boost_LIBRARIES} ${cpprest_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

find_package(SWIG REQUIRED)
include(UseSWIG)

set_property(SOURCE ds_api_python.i ds_api_csharp.i ds_api.i PROPERTY CPLUSPLUS ON)
SET(CMAKE_SWIG_OUTDIR "swig")

# TODO
# https://cmake.org/cmake/help/v3.8/release/3.8.html
# find_package( CSharp REQUIRED )
# enable_language(CSharp)

SWIG_ADD_MODULE(ds_cs csharp ds_api_csharp.i)
SWIG_LINK_LIBRARIES(ds_cs ${PROJECT_NAME})


add_subdirectory(python)

