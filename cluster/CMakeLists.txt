cmake_minimum_required (VERSION 3.5)
project (spanning_tree)

set (CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

include_directories(${vwcommon_INCLUDE_DIRS}) 

add_executable(${PROJECT_NAME} spanning_tree_main.cc spanning_tree.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE vwcommon Threads::Threads)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
