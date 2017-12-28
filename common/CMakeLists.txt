cmake_minimum_required (VERSION 3.5)
project (vwcommon)

set (CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} vw_exception.cc)

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
