set(vw_csv_parser_sources
    include/vw/csv_parser/parse_example_csv.h
    src/parse_example_csv.cc
)

vw_add_library(
    NAME "csv_parser"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_csv_parser_sources}
    PUBLIC_DEPS vw_common vw_config vw_core
    DESCRIPTION "Parser implementation that reads csv examples. Disabled by default. Enable with `VW_BUILD_CSV`"
    EXCEPTION_DESCRIPTION "Yes"
)

vw_add_test_executable(
  FOR_LIB "csv_parser"
  SOURCES "tests/csv_parser_test.cc"
  EXTRA_DEPS vw_core vw_test_common
)
