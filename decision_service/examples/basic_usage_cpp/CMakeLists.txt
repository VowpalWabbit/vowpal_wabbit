cmake_minimum_required (VERSION 3.5)
project (rl_basic_usage)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/../../include")

add_executable(${PROJECT_NAME} basic_usage_cpp.cc)

target_link_libraries(${PROJECT_NAME} PUBLIC rl ${Boost_LIBRARIES} ${BCRYPT_LIBRARY})
