set(vw_json_parser_sources
    include/vw/json_parser/decision_service_utils.h
    include/vw/json_parser/parse_example_json.h
    include/vw/json_parser/parse_example_slates_json.h
    src/json_utils.h
    src/parse_example_json.cc
    src/parse_example_slates_json.cc
)

vw_add_library(
    NAME "json_parser"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_json_parser_sources}
    PUBLIC_DEPS vw_common vw_core
    PRIVATE_DEPS $<BUILD_INTERFACE:RapidJSON>
    DESCRIPTION "Parser implementation that reads json and dsjson examples."
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)

vw_add_test_executable(
  FOR_LIB "json_parser"
  SOURCES "tests/json_parser_test.cc" "tests/dsjson_parser_test.cc"
  EXTRA_DEPS vw_core vw_test_common
)
