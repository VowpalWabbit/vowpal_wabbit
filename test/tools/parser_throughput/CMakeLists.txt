add_executable(parser_throughput main.cc)
target_link_libraries(parser_throughput PRIVATE VowpalWabbit::vw Boost::program_options)
