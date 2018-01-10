cmake_minimum_required (VERSION 3.5)

# Common configuration

set(PACKAGE_VERSION 8.5.0)

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "") 
endif ()

set(CMAKE_VW_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_VW_MODULE_PATH})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# option(VW_SSE2 "Enable SSE2 optimization" ON)
include(DiscoverPythonUnittest)

# set(Boost_VERSION 1.66.0)
# DSVM only has Boost 1.58.0 installed
set(Boost_VW_VERSION 1.58.0)

if(NOT Boost_USE_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS OFF)
endif()   

set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
set(THREADS_PREFER_PTHREAD_FLAG ON)
#set(OPENSSL_USE_STATIC_LIBS TRUE)


enable_testing()

# NOTE: The order matters! The most independent ones should go first.
add_subdirectory(common)
add_subdirectory(allreduce) 
add_subdirectory(vowpalwabbit) 
add_subdirectory(cluster) 
add_subdirectory(library) 
add_subdirectory(decision_service) 
add_subdirectory(python) 

# install(TARGETS ${PROJECT_NAME}
        # RUNTIME DESTINATION bin
        # LIBRARY DESTINATION lib
        # ARCHIVE DESTINATION lib/static
        # PUBLIC_HEADER DESTINATION include)
