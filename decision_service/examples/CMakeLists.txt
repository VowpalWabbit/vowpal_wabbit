cmake_minimum_required (VERSION 3.5)

find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

add_subdirectory(basic_usage_cpp) 
add_subdirectory(rl_sim_cpp)