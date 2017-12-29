cmake_minimum_required (VERSION 3.5)
project (spanning_tree)

set (CMAKE_CXX_STANDARD 11)


find_package(Threads REQUIRED)

include_directories(${vwcommon_INCLUDE_DIRS}) 

add_executable(${PROJECT_NAME} spanning_tree_main.cc spanning_tree.cc)

if(WIN32)
	set(ADDITIONAL_LIBRARY Ws2_32.lib)	
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE vwcommon Threads::Threads ${ADDITIONAL_LIBRARY})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
