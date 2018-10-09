cmake_minimum_required(VERSION 3.2)

set(PACKAGE_VERSION 8.6.1)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

find_package(ZLIB REQUIRED)
add_library(zlib UNKNOWN IMPORTED)
set_target_properties(zlib PROPERTIES
  IMPORTED_LOCATION "${ZLIB_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
)

set (CMAKE_CXX_STANDARD 11)

set(explore_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/explore")

# TODO: gcov, unclear how to implement

# include(DiscoverPythonUnittest)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(vowpalwabbit)

# add_subdirectory(cluster)
# add_subdirectory(library)
# TODO: rapidjson source generates too many warnings
add_subdirectory(doc)
# if(WIN32)
# #add_subdirectory(c_test)
# #add_subdirectory(cs)
# else()
# add_subdirectory(test)
# endif()
# add_subdirectory(python)
# add_subdirectory(java)
