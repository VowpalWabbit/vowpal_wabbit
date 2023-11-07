set(vw_cache_parser_sources
    include/vw/cache_parser/parse_example_cache.h
    src/parse_example_cache.cc
)

vw_add_library(
    NAME "cache_parser"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_cache_parser_sources}
    PUBLIC_DEPS vw_common vw_core
    DESCRIPTION "Read and write VW examples with internal cache format."
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)

vw_add_test_executable(
  FOR_LIB "cache_parser"
  SOURCES "tests/cache_test.cc"
  EXTRA_DEPS vw_core vw_io vw_test_common
)
