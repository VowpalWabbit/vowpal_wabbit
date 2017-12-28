cmake_minimum_required (VERSION 3.5)
project (library)

set (CMAKE_CXX_STANDARD 11)

include_directories(${vw_INCLUDE_DIRS}) 

add_executable(library_example library_example.cc)
target_link_libraries(library_example PRIVATE vw)

add_executable(ezexample_train ezexample_train.cc)
target_link_libraries(ezexample_train PRIVATE vw)

add_executable(ezexample_predict ezexample_predict.cc)
target_link_libraries(ezexample_predict PRIVATE vw)
