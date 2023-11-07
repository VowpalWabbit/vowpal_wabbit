add_executable(parser_throughput main.cc)
target_link_libraries(parser_throughput PRIVATE vw_core)

if(VW_FEAT_CSV)
  target_link_libraries(parser_throughput PRIVATE vw_csv_parser)
endif()