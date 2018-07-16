cmake_minimum_required (VERSION 3.5)
project (rl_sim_cpp)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/../../include")

add_executable(${PROJECT_NAME} main.cc person.cc rl_sim.cc)

target_link_libraries(${PROJECT_NAME} PUBLIC rl ${Boost_LIBRARIES} ${BCRYPT_LIBRARY})
