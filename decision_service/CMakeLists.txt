cmake_minimum_required (VERSION 3.5)
project (ds)

set (CMAKE_CXX_STANDARD 14)

# BOOST python
find_package(Boost 1.58.0 COMPONENTS regex)
find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIRS}) 
include_directories(${cpprest_INCLUDE_DIRS}) 
include_directories(${vw_INCLUDE_DIRS}) 

add_library(${PROJECT_NAME} ds_api.cc ds_vw.cc event_hub_client.cc)

target_link_libraries(ds PRIVATE vwlib ${Boost_LIBRARIES} ${cpprest_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)
