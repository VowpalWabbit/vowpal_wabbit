add_subdirectory(common)
add_subdirectory(slim)
if(BUILD_FLATBUFFERS)
  add_subdirectory(fb_parser)
endif()
configure_file(config.h.in config.h)
# Use position independent code for all targets in this directory
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(vw_io STATIC io/io_adapter.h io/io_adapter.cc io/logger.h io/logger.cc)
target_include_directories(vw_io PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(vw_io PUBLIC VowpalWabbit::common ${spdlog_target} fmt::fmt PRIVATE ZLIB::ZLIB)

if(SPDLOG_SYS_DEP)
  # this doesn't get defined when using a system-installed spdlog
  target_compile_definitions(vw_io PUBLIC SPDLOG_FMT_EXTERNAL)
endif()

add_library(VowpalWabbit::io ALIAS vw_io)

add_library(allreduce STATIC allreduce_sockets.cc allreduce_threads.cc)
target_include_directories(allreduce PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(VowpalWabbit::allreduce ALIAS allreduce)

target_link_libraries(allreduce PRIVATE vw_io)

# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(allreduce PUBLIC wsock32 ws2_32)
else()
  target_compile_options(allreduce PUBLIC ${linux_flags})
endif()

set(vw_all_headers
  accumulate.h
  action_score.h
  active_multiclass_prediction.h
  allreduce.h
  api_status.h
  array_parameters_dense.h
  array_parameters.h
  beam.h
  best_constant.h
  cache.h
  cached_learner.h
  cb_continuous_label.h
  cb_label_parser.h
  cb_type.h
  cb.h
  ccb_label.h
  ccb_reduction_features.h
  compat.h
  config/cli_help_formatter.h
  config/cli_options_serializer.h
  config/help_formatter.h
  config/option_group_definition.h
  config/option.h
  config/options_cli.h
  config/options_name_extractor.h
  config/options_serializer.h
  config/options.h
  constant.h
  continuous_actions_reduction_features.h
  correctedMath.h
  cost_sensitive.h
  crossplat_compat.h
  debug_log.h
  debug_print.h
  decision_scores.h
  distributionally_robust.h
  epsilon_reduction_features.h
  error_constants.h
  error_data.h
  error_reporting.h
  example_predict.h
  example.h
  fast_pow10.h
  feature_group.h
  gd_predict.h
  gen_cs_example.h
  generic_range.h
  global_data.h
  guard.h
  hashstring.h
  interactions_predict.h
  io_buf.h
  json_utils.h
  kskip_ngram_transformer.h
  label_dictionary.h
  label_parser.h
  label_type.h
  learner.h
  loss_functions.h
  memory.h
  metric_sink.h
  model_utils.h
  multiclass.h
  multilabel.h
  named_labels.h
  network.h
  no_label.h
  numeric_casts.h
  object_pool.h
  parse_args.h
  parse_dispatch_loop.h
  parse_example_json.h
  parse_example.h
  parse_primitives.h
  parse_regressor.h
  parse_slates_example_json.h
  parser.h
  prediction_type.h
  print_utils.h
  prob_dist_cont.h
  queue.h
  rand_state.h
  rand48.h
  reduction_features.h
  reduction_stack.h
  reductions_fwd.h
  reductions/active_cover.h
  reductions/active.h
  reductions/audit_regressor.h
  reductions/autolink.h
  reductions/automl.h
  reductions/baseline_challenger_cb.h
  reductions/baseline.h
  reductions/bfgs.h
  reductions/binary.h
  reductions/boosting.h
  reductions/bs.h
  reductions/cats_pdf.h
  reductions/cats_tree.h
  reductions/cats.h
  reductions/cb/cb_adf.h
  reductions/cb/cb_algs.h
  reductions/cb/cb_dro.h
  reductions/cb/cb_explore_adf_bag.h
  reductions/cb/cb_explore_adf_common.h
  reductions/cb/cb_explore_adf_cover.h
  reductions/cb/cb_explore_adf_first.h
  reductions/cb/cb_explore_adf_greedy.h
  reductions/cb/cb_explore_adf_regcb.h
  reductions/cb/cb_explore_adf_rnd.h
  reductions/cb/cb_explore_adf_softmax.h
  reductions/cb/cb_explore_adf_squarecb.h
  reductions/cb/cb_explore_adf_synthcover.h
  reductions/cb/cb_explore_pdf.h
  reductions/cb/cb_explore.h
  reductions/cb/cb_to_cb_adf.h
  reductions/cb/cbify.h
  reductions/cb/warm_cb.h
  reductions/cbzo.h
  reductions/classweight.h
  reductions/conditional_contextual_bandit.h
  reductions/confidence.h
  reductions/count_label.h
  reductions/cs_active.h
  reductions/csoaa_ldf.h
  reductions/csoaa.h
  reductions/ect.h
  reductions/explore_eval.h
  reductions/expreplay.h
  reductions/freegrad.h
  reductions/ftrl.h
  reductions/gd_mf.h
  reductions/gd.h
  reductions/generate_interactions.h
  reductions/get_pmf.h
  reductions/interact.h
  reductions/interaction_ground.h
  interactions.h
  reductions/kernel_svm.h
  reductions/lda_core.h
  reductions/log_multi.h
  reductions/lrq.h
  reductions/lrqfa.h
  reductions/marginal.h
  reductions/memory_tree.h
  reductions/metrics.h
  reductions/mf.h
  reductions/multilabel_oaa.h
  reductions/mwt.h
  reductions/nn.h
  reductions/noop.h
  reductions/oaa.h
  reductions/offset_tree.h
  reductions/oja_newton.h
  reductions/plt.h
  reductions/pmf_to_pdf.h
  reductions/print.h
  reductions/recall_tree.h
  reductions/sample_pdf.h
  reductions/scorer.h
  reductions/search/search_dep_parser.h
  reductions/search/search_entityrelationtask.h
  reductions/search/search_graph.h
  reductions/search/search_hooktask.h
  reductions/search/search_meta.h
  reductions/search/search_multiclasstask.h
  reductions/search/search_sequencetask.h
  reductions/search/search.h
  reductions/sender.h
  reductions/shared_feature_merger.h
  reductions/slates.h
  reductions/stagewise_poly.h
  reductions/svrg.h
  reductions/topk.h
  scope_exit.h
  scored_config.h
  shared_data.h
  simple_label_parser.h
  simple_label.h
  slates_label.h
  spanning_tree.h
  stable_unique.h
  tag_utils.h
  text_utils.h
  unique_sort.h
  v_array.h
  version.h
  vw_allreduce.h
  vw_math.h
  vw_validate.h
  vw_versions.h
  vw.h
  vwdll.h
  vwvis.h
)


if(BUILD_EXTERNAL_PARSER)
  set(vw_all_headers ${vw_all_headers} ${external_parser_headers})
endif()

set(vw_all_sources
  accumulate.cc
  action_score.cc
  api_status.cc
  best_constant.cc
  cache.cc
  cb_continuous_label.cc
  cb_type.cc
  cb.cc
  ccb_label.cc
  ccb_reduction_features.cc
  config/cli_help_formatter.cc
  config/cli_options_serializer.cc
  config/help_formatter.cc
  config/option_group_definition.cc
  config/options_cli.cc
  config/options_name_extractor.cc
  config/options.cc
  cost_sensitive.cc
  crossplat_compat.cc
  debug_print.cc
  decision_scores.cc
  distributionally_robust.cc
  example_predict.cc
  example.cc
  feature_group.cc
  gen_cs_example.cc
  global_data.cc
  hashstring.cc
  io_buf.cc
  kskip_ngram_transformer.cc
  label_dictionary.cc
  label_parser.cc
  label_type.cc
  learner.cc
  loss_functions.cc
  metric_sink.cc
  multiclass.cc
  multilabel.cc
  named_labels.cc
  network.cc
  no_label.cc
  parse_args.cc
  parse_example.cc
  parse_primitives.cc
  parse_regressor.cc
  parser.cc
  prediction_type.cc
  print_utils.cc
  prob_dist_cont.cc
  rand48.cc
  reduction_stack.cc
  reductions/active_cover.cc
  reductions/active.cc
  reductions/audit_regressor.cc
  reductions/autolink.cc
  reductions/automl.cc
  reductions/baseline_challenger_cb.cc
  reductions/baseline.cc
  reductions/bfgs.cc
  reductions/binary.cc
  reductions/boosting.cc
  reductions/bs.cc
  reductions/cats_pdf.cc
  reductions/cats_tree.cc
  reductions/cats.cc
  reductions/cb/cb_adf.cc
  reductions/cb/cb_algs.cc
  reductions/cb/cb_dro.cc
  reductions/cb/cb_explore_adf_bag.cc
  reductions/cb/cb_explore_adf_cover.cc
  reductions/cb/cb_explore_adf_first.cc
  reductions/cb/cb_explore_adf_greedy.cc
  reductions/cb/cb_explore_adf_regcb.cc
  reductions/cb/cb_explore_adf_rnd.cc
  reductions/cb/cb_explore_adf_softmax.cc
  reductions/cb/cb_explore_adf_squarecb.cc
  reductions/cb/cb_explore_adf_synthcover.cc
  reductions/cb/cb_explore_pdf.cc
  reductions/cb/cb_explore.cc
  reductions/cb/cb_sample.cc
  reductions/cb/cb_to_cb_adf.cc
  reductions/cb/cbify.cc
  reductions/cb/warm_cb.cc
  reductions/cbzo.cc
  reductions/classweight.cc
  reductions/conditional_contextual_bandit.cc
  reductions/confidence.cc
  reductions/count_label.cc
  reductions/cs_active.cc
  reductions/csoaa_ldf.cc
  reductions/csoaa.cc
  reductions/ect.cc
  reductions/explore_eval.cc
  reductions/freegrad.cc
  reductions/ftrl.cc
  reductions/gd_mf.cc
  reductions/gd.cc
  reductions/generate_interactions.cc
  reductions/get_pmf.cc
  reductions/interact.cc
  reductions/interaction_ground.cc
  interactions.cc
  reductions/kernel_svm.cc
  reductions/lda_core.cc
  reductions/log_multi.cc
  reductions/lrq.cc
  reductions/lrqfa.cc
  reductions/marginal.cc
  reductions/memory_tree.cc
  reductions/metrics.cc
  reductions/mf.cc
  reductions/multilabel_oaa.cc
  reductions/mwt.cc
  reductions/nn.cc
  reductions/noop.cc
  reductions/oaa.cc
  reductions/offset_tree.cc
  reductions/oja_newton.cc
  reductions/plt.cc
  reductions/pmf_to_pdf.cc
  reductions/print.cc
  reductions/recall_tree.cc
  reductions/sample_pdf.cc
  reductions/scorer.cc
  reductions/search/search_dep_parser.cc
  reductions/search/search_entityrelationtask.cc
  reductions/search/search_graph.cc
  reductions/search/search_hooktask.cc
  reductions/search/search_meta.cc
  reductions/search/search_multiclasstask.cc
  reductions/search/search_sequencetask.cc
  reductions/search/search.cc
  reductions/sender.cc
  reductions/shared_feature_merger.cc
  reductions/slates.cc
  reductions/stagewise_poly.cc
  reductions/svrg.cc
  reductions/topk.cc
  scored_config.cc
  shared_data.cc
  simple_label_parser.cc
  simple_label.cc
  slates_label.cc
  tag_utils.cc
  text_utils.cc
  unique_sort.cc
  version.cc
  vw_validate.cc
)

if(BUILD_EXTERNAL_PARSER)
  set(vw_all_sources ${vw_all_sources} ${external_parser_sources})
endif()

if (vw_BUILD_NET_FRAMEWORK)
set(vw_all_sources ${vw_all_sources} spanning_tree.cc)
endif()

add_library(vw STATIC ${vw_all_sources} ${vw_all_headers})

target_link_libraries(vw
  PUBLIC
    VowpalWabbit::common VowpalWabbit::explore VowpalWabbit::allreduce ${spdlog_target} fmt::fmt
  PRIVATE
    ${CMAKE_DL_LIBS} ${LINK_THREADS} vw_io
    # Workaround an issue where RapidJSON needed to be exported tom install the target. This is
    # actually a private dependency and so do not "link" when processing targets for installation.
    # https://gitlab.kitware.com/cmake/cmake/issues/15415
    $<BUILD_INTERFACE:RapidJSON>)

# If we are using a sys dep then we can link as normal as it is imported.
if(VW_BOOST_MATH_SYS_DEP)
  target_link_libraries(vw PRIVATE Boost::math)
else()
  # If using the vendored version we need to treat it differently.
  # CMake must propagate interface libs even if private in case they link
  # libraries. Since we know Boost::math does not link any further libs we
  # have to supply the build requirements piecewise and leave off link libraries.
  # https://gitlab.kitware.com/cmake/cmake/-/issues/15415
  target_include_directories(vw PRIVATE $<TARGET_PROPERTY:Boost::math,INTERFACE_INCLUDE_DIRECTORIES>)
  target_compile_options(vw PRIVATE $<TARGET_PROPERTY:Boost::math,INTERFACE_COMPILE_OPTIONS>)
  target_compile_definitions(vw PRIVATE $<TARGET_PROPERTY:Boost::math,INTERFACE_COMPILE_DEFINITIONS>)
endif()

if (BUILD_EXTERNAL_PARSER)
  target_compile_definitions(vw PUBLIC BUILD_EXTERNAL_PARSER)
  target_include_directories(vw PRIVATE ${EXTERNAL_PARSER_DIR})
  target_link_libraries(vw PRIVATE ${EXTERNAL_PARSER_LIBS})
endif()

if (BUILD_PRIVACY_ACTIVATION)
  target_compile_definitions(vw PUBLIC PRIVACY_ACTIVATION)
endif()

if(BUILD_FLATBUFFERS)
  target_link_libraries(vw PRIVATE VowpalWabbit::fb_parser)
  target_compile_definitions(vw PUBLIC BUILD_FLATBUFFERS)
endif()

add_library(VowpalWabbit::vw ALIAS vw)

if(GCOV)
  target_link_libraries(vw PUBLIC gcov --coverage)
endif()

target_include_directories(vw PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_definitions(vw PUBLIC _FILE_OFFSET_BITS=64 $<$<CONFIG:RELEASE>:NDEBUG>)
# Only define if Clang is not used
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw PUBLIC __extern_always_inline=inline)
endif()

# Add natvis file if the Generator is Visual Studio
if (MSVC_IDE)
  target_sources(vw PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/vw_types.natvis> )
endif()

# Clang-cl on Windows has issues with our usage of SIMD types. Turn it off explicitly for Windows + clang-cl to mitigate.
# See issue #
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw PUBLIC VW_NO_INLINE_SIMD)
endif()

# TODO code analysis
if(WIN32)
  target_compile_definitions(vw PUBLIC __SSE2__)
  target_compile_options(vw PUBLIC /MP /Zc:__cplusplus)
else()
  target_compile_options(vw PUBLIC ${linux_flags})
endif()

# Turn on warnings
if(WARNINGS)
  if(MSVC)
    target_compile_options(vw PRIVATE /W4)
  else(MSVC)
    target_compile_options(vw PRIVATE -Wall -Wextra -Wpedantic)
  endif(MSVC)
endif(WARNINGS)

# Turn on warnings as errors
if(WARNING_AS_ERROR)
  if(MSVC)
    target_compile_options(vw PRIVATE /WX)
  else(MSVC)
    target_compile_options(vw PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif(MSVC)
endif(WARNING_AS_ERROR)

if(NOT WIN32)
  add_executable(active_interactor active_interactor.cc)

  if(GCOV)
    target_link_libraries(active_interactor PUBLIC gcov --coverage)
  endif()

  if(VW_INSTALL)
    install(
      TARGETS active_interactor
      EXPORT VowpalWabbitConfig
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()

  target_compile_definitions(active_interactor PUBLIC _FILE_OFFSET_BITS=64)
  target_compile_definitions(active_interactor PUBLIC $<$<CONFIG:RELEASE>:NDEBUG>)
  target_compile_options(active_interactor PUBLIC ${linux_flags})

  find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)
  if (HELP2MAN_EXECUTABLE)
    add_custom_target(manpage ALL
      COMMAND ${HELP2MAN_EXECUTABLE}
        --no-info
        --name="Vowpal Wabbit -- fast online learning tool" $<TARGET_FILE:vw-bin>
        --output=$<TARGET_FILE:vw-bin>.1
      DEPENDS vw-bin)

      if(VW_INSTALL)
        install(FILES $<TARGET_FILE:vw-bin>.1 DESTINATION share/man/man1)
      endif()
  else()
    message(STATUS "help2man not found, please install it to generate manpages")
  endif()
endif()

# build main executable
add_executable(vw-bin main.cc)
target_link_libraries(vw-bin PRIVATE VowpalWabbit::vw)
set_target_properties(vw-bin PROPERTIES OUTPUT_NAME vw)

if(STATIC_LINK_VW)
  target_link_libraries(vw-bin PRIVATE ${unix_static_flag})
endif()

if(WIN32)
  set_target_properties(vw_io PROPERTIES DEBUG_POSTFIX d)
  set_target_properties(allreduce PROPERTIES DEBUG_POSTFIX d)
  set_target_properties(vw PROPERTIES DEBUG_POSTFIX d)
endif()

# This is used by Travis to not build vw_c_wrapper, this required <codecvt> which became available in GCC 5.1
if(NOT DEFINED DO_NOT_BUILD_VW_C_WRAPPER)
  if(STATIC_LINK_VW)
    add_library(vw_c_wrapper vwdll.cc)
  else()
    add_library(vw_c_wrapper SHARED vwdll.cc)
  endif()

  add_library(VowpalWabbit::vw_c_wrapper ALIAS vw_c_wrapper)

  target_compile_definitions(vw_c_wrapper PUBLIC VWDLL_EXPORTS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE ZLIB_WINAPI)
  target_link_libraries(vw_c_wrapper PUBLIC VowpalWabbit::vw)
  if(VW_INSTALL)
    install(
      TARGETS vw_c_wrapper
      EXPORT VowpalWabbitConfig
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()

if(VW_INSTALL)
  install(
    TARGETS vw vw-bin allreduce vw_io
    EXPORT VowpalWabbitConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN ".nuget" EXCLUDE
    PATTERN "slim" EXCLUDE
    PATTERN "win32" EXCLUDE
    PATTERN "out" EXCLUDE
    PATTERN "packages" EXCLUDE
    PATTERN "x64" EXCLUDE
    PATTERN ".vs" EXCLUDE
  )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit
  )
endif()
