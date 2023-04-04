add_executable(parser_throughput main.cc)
target_link_libraries(parser_throughput PRIVATE vw_core)

if(VW_FEATURE_CSV)
  target_link_libraries(parser_throughput PRIVATE vw_csv_parser)

  # Adding explicitly since this doesn't use vw_add_executable
  target_compile_definitions(parser_throughput PUBLIC VW_FEATURE_CSV_ENABLED)
endif()