set(CMAKE_CXX_STANDARD 11)

find_package( Boost 1.40 COMPONENTS program_options REQUIRED)
find_package(ZLIB REQUIRED)

include_directories( ${Boost_INCLUDE_DIR})
include_directories("../../explore" "../../vowpalwabbit")

configure_file(../../vowpalwabbit/config.h.in config.h)

add_executable(vwtxt_to_flatbuf txt_to_flat.cc ${vw_all_sources} ${vw_all_headers} ${flat_headers})
target_link_libraries(vwtxt_to_flatbuf vw allreduce Boost::boost Boost::program_options ZLIB::ZLIB)