set(CMAKE_CXX_STANDARD 11)

find_package( Boost 1.40 COMPONENTS program_options REQUIRED)
find_package(ZLIB REQUIRED)

include_directories( ${Boost_INCLUDE_DIR})
include_directories("../../explore" "../../vowpalwabbit")

configure_file(../../vowpalwabbit/config.h.in config.h)

add_executable(to_flatbuff txt_to_flat.cc vw_to_flat.cc vw_to_flat.h)
target_include_directories(to_flatbuff PRIVATE $<TARGET_PROPERTY:vw,INCLUDE_DIRECTORIES>)
#add_dependencies(to_flatbuff fbschemas)
target_link_libraries(to_flatbuff PRIVATE vw allreduce Boost::boost Boost::program_options ZLIB::ZLIB)