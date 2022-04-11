add_executable(parser_throughput main.cc)

# We are using a "private" header in VW and therefore must bring in the RapidJSON dependency manually.
target_link_libraries(parser_throughput PRIVATE VowpalWabbit::config RapidJSON VowpalWabbit::vw)
