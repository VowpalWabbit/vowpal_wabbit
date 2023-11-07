set(VW_SLIM_SOURCES
  src/example_predict_builder.cc
  src/model_parser.cc
  src/opts.cc
  src/vw_slim_predict.cc
  ../core/src/feature_group.cc
  ../core/src/example_predict.cc
  ../core/src/array_parameters_dense.cc
  ../core/src/array_parameters_sparse.cc
  ../core/src/interactions.cc
)

set(VW_SLIM_HEADERS
  include/vw/slim/example_predict_builder.h
  include/vw/slim/model_parser.h
  include/vw/slim/opts.h
  include/vw/slim/vw_slim_predict.h
  include/vw/slim/vw_slim_return_codes.h
)

# If we want slim to be installable it probably needs to be via a component so it can be separate to the main installation
vw_add_library(
  NAME "slim"
  TYPE "STATIC_ONLY"
  SOURCES ${VW_SLIM_SOURCES} ${VW_SLIM_HEADERS}
  PUBLIC_DEPS vw_common vw_explore
  DESCRIPTION "Minimal inference only runtime"
  EXCEPTION_DESCRIPTION "No"
)
target_compile_definitions(vw_slim PUBLIC EXPLORE_NOEXCEPT VW_NOEXCEPT)

# TODO - remove this break of component boundaries at some point
# At least update when the vowpalwabbit include dir is not used anymore
target_include_directories(vw_slim PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../core/include
)

set(vw_slim_test_sources
  test/data.h
  test/ut_explore.cc
  test/ut_opts.cc
  test/ut_util.cc
)

vw_add_test_executable(
  FOR_LIB "slim"
  SOURCES ${vw_slim_test_sources}
  COMPILE_DEFS VW_SLIM_TEST_DIR="${CMAKE_CURRENT_LIST_DIR}/test/"
)
