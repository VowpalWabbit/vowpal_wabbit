set(VW_SLIM_SOURCES
  src/example_predict_builder.cc
  src/model_parser.cc
  src/opts.cc
  src/vw_slim_predict.cc
  ../feature_group.cc
  ../example_predict.cc
  ../interactions.cc
)

set(VW_SLIM_HEADERS
  include/vw/slim/example_predict_builder.h
  include/vw/slim/model_parser.h
  include/vw/slim/opts.h
  include/vw/slim/vw_slim_predict.h
  include/vw/slim/vw_slim_return_codes.h
)

vw_add_library(
  NAME "slim"
  TYPE "STATIC_ONLY"
  SOURCES ${VW_SLIM_SOURCES} ${VW_SLIM_HEADERS}
  PUBLIC_DEPS VowpalWabbit::common VowpalWabbit::explore
  ENABLE_INSTALL
)
target_compile_definitions(vw_slim PUBLIC EXPLORE_NOEXCEPT VW_NOEXCEPT)

# TODO - remove this break of component boundaries at some point
target_include_directories(vw_slim PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../)

set(vw_slim_test_sources
  test/data.h
  test/ut_explore.cc
  test/ut_opts.cc
  test/ut_util.h
  test/ut_util.cc
)

vw_add_test_executable(
  FOR_LIB "slim"
  SOURCES ${vw_slim_test_sources}
)
