add_library(vwslim
  example_predict_builder.cc
  model_parser.cc
  opts.cc
  vw_slim_predict.cc
  ../../example_predict.cc)

target_include_directories(vwslim PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../)
target_compile_definitions(vwslim PUBLIC EXPLORE_NOEXCEPT VW_NOEXCEPT)
target_link_libraries(vwslim PUBLIC VowpalWabbit::explore)

include(GNUInstallDirs)

install(
  TARGETS vwslim
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
