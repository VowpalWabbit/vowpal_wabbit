add_executable(parser_throughput main.cc)
target_link_libraries(parser_throughput PRIVATE vw_core)

if(VW_FEATURE_CSV_ENABLED)
  target_link_libraries(parser_throughput PRIVATE vw_csv_parser)
endif()