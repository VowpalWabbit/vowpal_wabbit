cmake_minimum_required (VERSION 3.7)

set(PACKAGE_VERSION 8.6.1)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

find_package(ZLIB REQUIRED)
add_library(zlib UNKNOWN IMPORTED)
set_target_properties(zlib PROPERTIES
  IMPORTED_LOCATION "${ZLIB_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
)

set(explore_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/explore")

# TODO: gcov, unclear how to implement
# option(VW_SSE2 "Enable SSE2 optimization" ON)
# include(DiscoverPythonUnittest)

# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_MULTITHREADED ON)
# set(Boost_USE_STATIC_RUNTIME OFF)

# set(THREADS_PREFER_PTHREAD_FLAG ON)

add_subdirectory(vowpalwabbit)

# add_subdirectory(cluster)
# add_subdirectory(library)
# add_subdirectory(decision_service)
# TODO: rapidjson source generates too many warnings
# add_subdirectory(doc)
# if(WIN32)
# #add_subdirectory(c_test)
# #add_subdirectory(cs)
# else()
# add_subdirectory(test)
# endif()
# add_subdirectory(python)
# add_subdirectory(java)
