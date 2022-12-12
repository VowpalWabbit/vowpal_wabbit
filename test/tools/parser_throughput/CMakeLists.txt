add_executable(parser_throughput main.cc)
target_link_libraries(parser_throughput PRIVATE vw_core)

if(VW_BUILD_CSV)
  target_link_libraries(parser_throughput PRIVATE vw_csv_parser)
  target_compile_definitions(parser_throughput PUBLIC VW_BUILD_CSV)
endif()