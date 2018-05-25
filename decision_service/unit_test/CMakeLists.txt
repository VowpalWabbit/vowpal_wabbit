cmake_minimum_required (VERSION 3.5)
project (rl_test)

set(CMAKE_CXX_STANDARD 11)

enable_testing()

include_directories(${rl_INCLUDE_DIRS})

# make sure we can do white-box testing
include_directories("${PROJECT_SOURCE_DIR}/../include")
include_directories(${vw_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/../rlclientlib")

find_package(Boost COMPONENTS unit_test_framework system REQUIRED)
add_executable(${PROJECT_NAME} main.cc async_batcher_test.cc callback_test.cc concurrent_queue_test.cc event_test.cc eventhub_client_test.cc 
	live_model_test.cc object_pool_test.cc ranking_response_test.cc str_util_test.cc
	safe_vw_test.cc
	http_server/http_server.cc)

if(MSVC)
    find_library(BCRYPT_LIBRARY bcrypt)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC rl ${Boost_LIBRARIES} ${BCRYPT_LIBRARY})

add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)

if(MSVC)
add_custom_target(check
	COMMAND set "PATH=%PATH%;${cpprest_LIBRARY}/../../bin"
	COMMAND ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
                  DEPENDS ${PROJECT_NAME})
else()
add_custom_target(check
	COMMAND ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
                  DEPENDS ${PROJECT_NAME})
endif()
