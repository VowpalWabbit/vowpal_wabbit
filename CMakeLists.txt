cmake_minimum_required (VERSION 3.5)

set(PACKAGE_VERSION 8.5.0)

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "") 
endif ()

set(CMAKE_VW_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_VW_MODULE_PATH})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# TODO: gcov, unclear how to implement

# option(VW_SSE2 "Enable SSE2 optimization" ON)
# include(DiscoverPythonUnittest)
include(InstallDependencies)

set(explore_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/explore")

if(NOT Boost_USE_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS OFF)
endif()   

set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_subdirectory(vowpalwabbit) 
add_subdirectory(cluster) 
add_subdirectory(library) 
add_subdirectory(decision_service) 

# TODO: rapidjson source generates too many warnings
add_subdirectory(doc) 

if(WIN32)
 #add_subdirectory(c_test) 
 #add_subdirectory(cs) 
else()
  add_subdirectory(test)
endif()

add_subdirectory(python)
add_subdirectory(java)