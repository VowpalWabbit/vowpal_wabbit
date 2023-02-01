set(vw_core_headers
  include/vw/core/accumulate.h
  include/vw/core/action_score.h
  include/vw/core/active_multiclass_prediction.h
  include/vw/core/api_status.h
  include/vw/core/array_parameters_dense.h
  include/vw/core/array_parameters_sparse.h
  include/vw/core/array_parameters.h
  include/vw/core/automl_impl.h
  include/vw/core/best_constant.h
  include/vw/core/cache.h
  include/vw/core/cached_learner.h
  include/vw/core/cb_continuous_label.h
  include/vw/core/cb_graph_feedback_reduction_features.h
  include/vw/core/multi_ex.h
  include/vw/core/learner_fwd.h
  include/vw/core/daemon_utils.h
  include/vw/core/cb_type.h
  include/vw/core/cb.h
  include/vw/core/ccb_label.h
  include/vw/core/ccb_reduction_features.h
  include/vw/core/estimators/confidence_sequence.h
  include/vw/core/estimators/confidence_sequence_robust.h
  include/vw/core/constant.h
  include/vw/core/continuous_actions_reduction_features.h
  include/vw/core/correctedMath.h
  include/vw/core/cost_sensitive.h
  include/vw/core/estimators/cressieread.h
  include/vw/core/crossplat_compat.h
  include/vw/core/debug_log.h
  include/vw/core/debug_print.h
  include/vw/core/decision_scores.h
  include/vw/core/estimators/distributionally_robust.h
  include/vw/core/epsilon_reduction_features.h
  include/vw/core/error_constants.h
  include/vw/core/error_data.h
  include/vw/core/error_reporting.h
  include/vw/core/example_predict.h
  include/vw/core/example.h
  include/vw/core/fast_pow10.h
  include/vw/core/feature_group.h
  include/vw/core/gd_predict.h
  include/vw/core/gen_cs_example.h
  include/vw/core/large_action_space_reduction_features.h
  include/vw/core/generic_range.h
  include/vw/core/global_data.h
  include/vw/core/guard.h
  include/vw/core/hashstring.h
  include/vw/core/interactions_predict.h
  include/vw/core/interactions.h
  include/vw/core/io_buf.h
  include/vw/core/kskip_ngram_transformer.h
  include/vw/core/label_dictionary.h
  include/vw/core/label_parser.h
  include/vw/core/label_type.h
  include/vw/core/learner.h
  include/vw/core/loss_functions.h
  include/vw/core/memory.h
  include/vw/core/merge.h
  include/vw/core/metrics_collector.h
  include/vw/core/metric_sink.h
  include/vw/core/model_utils.h
  include/vw/core/multi_model_utils.h
  include/vw/core/multiclass.h
  include/vw/core/multilabel.h
  include/vw/core/named_labels.h
  include/vw/core/network.h
  include/vw/core/no_label.h
  include/vw/core/numeric_casts.h
  include/vw/core/object_pool.h
  include/vw/core/parse_args.h
  include/vw/core/parse_dispatch_loop.h
  include/vw/core/parse_example_json.h
  include/vw/core/parse_example.h
  include/vw/core/parse_primitives.h
  include/vw/core/parse_regressor.h
  include/vw/core/parse_slates_example_json.h
  include/vw/core/parser.h
  include/vw/core/prediction_type.h
  include/vw/core/print_utils.h
  include/vw/core/prob_dist_cont.h
  include/vw/core/queue.h
  include/vw/core/rand_state.h
  include/vw/core/reduction_features.h
  include/vw/core/reduction_stack.h
  include/vw/core/reductions/active_cover.h
  include/vw/core/reductions/active.h
  include/vw/core/reductions/audit_regressor.h
  include/vw/core/reductions/autolink.h
  include/vw/core/reductions/automl.h
  include/vw/core/reductions/baseline_challenger_cb.h
  include/vw/core/reductions/baseline.h
  include/vw/core/reductions/bfgs.h
  include/vw/core/reductions/binary.h
  include/vw/core/reductions/boosting.h
  include/vw/core/reductions/bs.h
  include/vw/core/reductions/cats_pdf.h
  include/vw/core/reductions/cats_tree.h
  include/vw/core/reductions/cats.h
  include/vw/core/reductions/cb/cb_actions_mask.h
  include/vw/core/reductions/cb/cb_adf.h
  include/vw/core/reductions/cb/cb_algs.h
  include/vw/core/reductions/cb/cb_dro.h
  include/vw/core/reductions/cb/cb_explore_adf_bag.h
  include/vw/core/reductions/cb/cb_explore_adf_common.h
  include/vw/core/reductions/cb/cb_explore_adf_cover.h
  include/vw/core/reductions/cb/cb_explore_adf_first.h
  include/vw/core/reductions/cb/cb_explore_adf_greedy.h
  include/vw/core/reductions/cb/cb_explore_adf_large_action_space.h
  include/vw/core/reductions/cb/cb_explore_adf_regcb.h
  include/vw/core/reductions/cb/cb_explore_adf_rnd.h
  include/vw/core/reductions/cb/cb_explore_adf_softmax.h
  include/vw/core/reductions/cb/cb_explore_adf_squarecb.h
  include/vw/core/reductions/cb/cb_explore_adf_synthcover.h
  include/vw/core/reductions/cb/cb_explore_pdf.h
  include/vw/core/reductions/cb/cb_explore.h
  include/vw/core/reductions/cb/cb_to_cb_adf.h
  include/vw/core/reductions/cb/cbify.h
  include/vw/core/reductions/cb/warm_cb.h
  include/vw/core/reductions/cbzo.h
  include/vw/core/reductions/classweight.h
  include/vw/core/reductions/conditional_contextual_bandit.h
  include/vw/core/reductions/confidence.h
  include/vw/core/reductions/count_label.h
  include/vw/core/reductions/cs_active.h
  include/vw/core/reductions/csoaa_ldf.h
  include/vw/core/reductions/csoaa.h
  include/vw/core/reductions/ect.h
  include/vw/core/reductions/eigen_memory_tree.h
  include/vw/core/reductions/epsilon_decay.h
  include/vw/core/reductions/explore_eval.h
  include/vw/core/reductions/expreplay.h
  include/vw/core/reductions/freegrad.h
  include/vw/core/reductions/ftrl.h
  include/vw/core/reductions/gd_mf.h
  include/vw/core/reductions/gd.h
  include/vw/core/reductions/generate_interactions.h
  include/vw/core/reductions/get_pmf.h
  include/vw/core/reductions/interact.h
  include/vw/core/reductions/interaction_ground.h
  include/vw/core/reductions/kernel_svm.h
  include/vw/core/reductions/lda_core.h
  include/vw/core/reductions/log_multi.h
  include/vw/core/reductions/lrq.h
  include/vw/core/reductions/lrqfa.h
  include/vw/core/reductions/marginal.h
  include/vw/core/reductions/memory_tree.h
  include/vw/core/reductions/metrics.h
  include/vw/core/reductions/mf.h
  include/vw/core/reductions/multilabel_oaa.h
  include/vw/core/reductions/mwt.h
  include/vw/core/reductions/nn.h
  include/vw/core/reductions/noop.h
  include/vw/core/reductions/oaa.h
  include/vw/core/reductions/offset_tree.h
  include/vw/core/reductions/oja_newton.h
  include/vw/core/reductions/plt.h
  include/vw/core/reductions/pmf_to_pdf.h
  include/vw/core/reductions/print.h
  include/vw/core/reductions/recall_tree.h
  include/vw/core/reductions/sample_pdf.h
  include/vw/core/reductions/scorer.h
  include/vw/core/reductions/search/search_dep_parser.h
  include/vw/core/reductions/search/search_entityrelationtask.h
  include/vw/core/reductions/search/search_graph.h
  include/vw/core/reductions/search/search_hooktask.h
  include/vw/core/reductions/search/search_meta.h
  include/vw/core/reductions/search/search_multiclasstask.h
  include/vw/core/reductions/search/search_sequencetask.h
  include/vw/core/reductions/search/search.h
  include/vw/core/reductions/sender.h
  include/vw/core/reductions/shared_feature_merger.h
  include/vw/core/reductions/slates.h
  include/vw/core/reductions/stagewise_poly.h
  include/vw/core/reductions/svrg.h
  include/vw/core/reductions/topk.h
  include/vw/core/scope_exit.h
  include/vw/core/shared_data.h
  include/vw/core/simple_label_parser.h
  include/vw/core/simple_label.h
  include/vw/core/slates_label.h
  include/vw/core/tag_utils.h
  include/vw/core/text_utils.h
  include/vw/core/thread_pool.h
  include/vw/core/unique_sort.h
  include/vw/core/v_array.h
  include/vw/core/version.h
  include/vw/core/vw_allreduce.h
  include/vw/core/vw_math.h
  include/vw/core/vw_validate.h
  include/vw/core/vw_versions.h
  include/vw/core/vw.h
  src/qr_decomposition.h
)

set(vw_core_sources
  src/accumulate.cc
  src/action_score.cc
  src/api_status.cc
  src/array_parameters_dense.cc
  src/array_parameters_sparse.cc
  src/best_constant.cc
  src/cb_continuous_label.cc
  src/cb_type.cc
  src/cb.cc
  src/ccb_label.cc
  src/ccb_reduction_features.cc
  src/confidence_sequence.cc
  src/confidence_sequence_robust.cc
  src/cost_sensitive.cc
  src/cressieread.cc
  src/crossplat_compat.cc
  src/debug_print.cc
  src/decision_scores.cc
  src/distributionally_robust.cc
  src/example_predict.cc
  src/example.cc
  src/feature_group.cc
  src/gen_cs_example.cc
  src/global_data.cc
  src/hashstring.cc
  src/interactions.cc
  src/io_buf.cc
  src/kskip_ngram_transformer.cc
  src/label_dictionary.cc
  src/label_parser.cc
  src/label_parser.cc
  src/label_type.cc
  src/learner.cc
  src/loss_functions.cc
  src/merge.cc
  src/metrics_collector.cc
  src/metric_sink.cc
  src/multiclass.cc
  src/multilabel.cc
  src/named_labels.cc
  src/network.cc
  src/no_label.cc
  src/parse_args.cc
  src/parse_example_json.cc
  src/parse_primitives.cc
  src/parse_regressor.cc
  src/parse_slates_example_json.cc
  src/parser.cc
  src/prediction_type.cc
  src/print_utils.cc
  src/prob_dist_cont.cc
  src/qr_decomposition.cc
  src/reduction_stack.cc
  src/reductions/active_cover.cc
  src/reductions/active.cc
  src/reductions/audit_regressor.cc
  src/reductions/autolink.cc
  src/reductions/automl.cc
  src/reductions/baseline_challenger_cb.cc
  src/reductions/baseline.cc
  src/reductions/bfgs.cc
  src/reductions/binary.cc
  src/reductions/boosting.cc
  src/reductions/bs.cc
  src/reductions/cats_pdf.cc
  src/reductions/cats_tree.cc
  src/reductions/cats.cc
  src/reductions/cb/details/large_action/compute_dot_prod_avx2.cc
  src/reductions/cb/details/large_action/compute_dot_prod_avx512.cc
  src/reductions/cb/details/large_action/one_pass_svd_impl.cc
  src/reductions/cb/details/large_action/one_rank_spanner_impl.cc
  src/reductions/cb/details/large_action/two_pass_svd_impl.cc
  src/reductions/cb/cb_actions_mask.cc
  src/reductions/cb/cb_adf.cc
  src/reductions/cb/cb_algs.cc
  src/reductions/cb/cb_dro.cc
  src/reductions/cb/cb_explore_adf_bag.cc
  src/reductions/cb/cb_explore_adf_cover.cc
  src/reductions/cb/cb_explore_adf_first.cc
  src/reductions/cb/cb_explore_adf_greedy.cc
  src/reductions/cb/cb_explore_adf_large_action_space.cc
  src/reductions/cb/cb_explore_adf_regcb.cc
  src/reductions/cb/cb_explore_adf_rnd.cc
  src/reductions/cb/cb_explore_adf_softmax.cc
  src/reductions/cb/cb_explore_adf_squarecb.cc
  src/reductions/cb/cb_explore_adf_synthcover.cc
  src/reductions/cb/cb_explore_pdf.cc
  src/reductions/cb/cb_explore.cc
  src/reductions/cb/cb_sample.cc
  src/reductions/cb/cb_to_cb_adf.cc
  src/reductions/cb/cbify.cc
  src/reductions/cb/warm_cb.cc
  src/reductions/cbzo.cc
  src/reductions/classweight.cc
  src/reductions/conditional_contextual_bandit.cc
  src/reductions/confidence.cc
  src/reductions/count_label.cc
  src/reductions/cs_active.cc
  src/reductions/csoaa_ldf.cc
  src/reductions/csoaa.cc
  src/reductions/details/automl/automl_impl.cc
  src/reductions/details/automl/automl_iomodel.cc
  src/reductions/details/automl/automl_oracle.cc
  src/reductions/details/automl/automl_util.cc
  src/reductions/ect.cc
  src/reductions/eigen_memory_tree.cc
  src/daemon_utils.cc
  src/reductions/epsilon_decay.cc
  src/reductions/explore_eval.cc
  src/reductions/freegrad.cc
  src/reductions/ftrl.cc
  src/reductions/gd_mf.cc
  src/reductions/gd.cc
  src/reductions/generate_interactions.cc
  src/reductions/get_pmf.cc
  src/reductions/interact.cc
  src/reductions/interaction_ground.cc
  src/reductions/kernel_svm.cc
  src/reductions/lda_core.cc
  src/reductions/log_multi.cc
  src/reductions/lrq.cc
  src/reductions/lrqfa.cc
  src/reductions/marginal.cc
  src/reductions/memory_tree.cc
  src/reductions/metrics.cc
  src/reductions/mf.cc
  src/reductions/multilabel_oaa.cc
  src/reductions/mwt.cc
  src/reductions/nn.cc
  src/reductions/noop.cc
  src/reductions/oaa.cc
  src/reductions/offset_tree.cc
  src/reductions/oja_newton.cc
  src/reductions/plt.cc
  src/reductions/pmf_to_pdf.cc
  src/reductions/print.cc
  src/reductions/recall_tree.cc
  src/reductions/sample_pdf.cc
  src/reductions/scorer.cc
  src/reductions/search/search_dep_parser.cc
  src/reductions/search/search_entityrelationtask.cc
  src/reductions/search/search_graph.cc
  src/reductions/search/search_hooktask.cc
  src/reductions/search/search_meta.cc
  src/reductions/search/search_multiclasstask.cc
  src/reductions/search/search_sequencetask.cc
  src/reductions/search/search.cc
  src/reductions/sender.cc
  src/reductions/shared_feature_merger.cc
  src/reductions/slates.cc
  src/reductions/stagewise_poly.cc
  src/reductions/svrg.cc
  src/reductions/topk.cc
  src/shared_data.cc
  src/simple_label_parser.cc
  src/simple_label.cc
  src/slates_label.cc
  src/tag_utils.cc
  src/text_utils.cc
  src/unique_sort.cc
  src/version.cc
  src/vw_validate.cc
  src/vw.cc
)

vw_add_library(
    NAME "core"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_core_headers} ${vw_core_sources}
    PUBLIC_DEPS vw_common vw_explore vw_allreduce vw_config fmt::fmt vw_cache_parser vw_text_parser vw_json_parser
    PRIVATE_DEPS
        # Special case for dealing with header-only INTERFACE dependencies of a STATIC library
        # Use BUILD_INTERFACE to prevent them from being exported, i.e. treat them as PRIVATE
        # https://gitlab.kitware.com/cmake/cmake/issues/15415
        ${CMAKE_DL_LIBS} ${LINK_THREADS} vw_io $<BUILD_INTERFACE:${boost_math_target}> $<BUILD_INTERFACE:RapidJSON>
        $<BUILD_INTERFACE:eigen> $<BUILD_INTERFACE:sse2neon>
    DESCRIPTION "This contains all remaining VW code, all reduction implementations, driver, option handling"
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)

target_include_directories(vw_core PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

if (VW_BUILD_LAS_WITH_SIMD AND (UNIX AND NOT APPLE) AND (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"))
  set_source_files_properties(src/reductions/cb/details/large_action/compute_dot_prod_avx2.cc PROPERTIES COMPILE_FLAGS "-mfma -mavx2")
  set_source_files_properties(src/reductions/cb/details/large_action/compute_dot_prod_avx512.cc PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512bw -mavx512vl -mavx512vpopcntdq")
  target_compile_definitions(vw_core PUBLIC BUILD_LAS_WITH_SIMD)
endif()

if(VW_BUILD_CSV)
  target_link_libraries(vw_core PRIVATE vw_csv_parser)
  target_compile_definitions(vw_core PUBLIC VW_BUILD_CSV)
endif()

if(BUILD_FLATBUFFERS)
  target_link_libraries(vw_core PRIVATE vw_fb_parser)
  target_compile_definitions(vw_core PUBLIC BUILD_FLATBUFFERS)
endif()

# Handle generated header
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/vw/core/config.h)
target_include_directories(vw_core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>)
if(VW_INSTALL)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

# Only define if Clang is not used
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw_core PUBLIC __extern_always_inline=inline)
endif()

if (MSVC_IDE)
  target_sources(vw_core PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/vw_types.natvis> )
endif()

# Clang-cl on Windows has issues with our usage of SIMD types. Turn it off explicitly for Windows + clang-cl to mitigate.
# See issue #
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw_core PUBLIC VW_NO_INLINE_SIMD)
endif()

if(WIN32)
  target_compile_definitions(vw_core PUBLIC __SSE2__)
endif()

vw_add_test_executable(
    FOR_LIB "core"
    EXTRA_DEPS vw_test_common
    SOURCES
      tests/automl_test.cc
      tests/automl_weights_test.cc
      tests/baseline_cb_test.cc
      tests/cats_test.cc
      tests/cats_tree_test.cc
      tests/cats_user_provided_pdf.cc
      tests/cb_explore_adf_test.cc
      tests/cb_graph_feedback_label_text_parse_test.cc
      tests/cb_large_actions_test.cc
      tests/cb_las_one_pass_svd_test.cc
      tests/cb_las_spanner_test.cc
      tests/ccb_parser_test.cc
      tests/ccb_test.cc
      tests/chain_hashing.cc
      tests/confidence_sequence_robust_test.cc
      tests/confidence_sequence_test.cc
      tests/continuous_actions_parser_test.cc
      tests/custom_reduction_test.cc
      tests/distributionally_robust_test.cc
      tests/eigen_memory_tree_test.cc
      tests/epsilon_decay_test.cc
      tests/epsilon_test.cc
      tests/example_header_test.cc
      tests/example_test.cc
      tests/feature_group_test.cc
      tests/flat_example_test.cc
      tests/guard_test.cc
      tests/interactions_test.cc
      tests/loss_functions_test.cc
      tests/math_test.cc
      tests/merge_header_opts_test.cc
      tests/merge_test.cc
      tests/minimal_custom_reduction.cc
      tests/model_util_test.cc
      tests/multiclass_label_parser_test.cc
      tests/numeric_cast_test.cc
      tests/object_pool_test.cc
      tests/offset_tree_test.cc
      tests/parse_args_test.cc
      tests/parser_test.cc
      tests/pmf_to_pdf_test.cc
      tests/power_test.cc
      tests/prediction_test.cc
      tests/random_test.cc
      tests/save_load_test.cc
      tests/scope_exit_test.cc
      tests/simulator.cc
      tests/simulator.h
      tests/slates_parser_test.cc
      tests/slates_test.cc
      tests/status_builder_test.cc
      tests/tag_utils_test.cc
      tests/thread_pool_test.cc
      tests/tokenize_test.cc
      tests/tutorial_test.cc
      tests/v_array_test.cc
      tests/vw_versions_test.cc
      tests/weights_test.cc
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  # Tests are allowed to access private headers.
  target_include_directories(vw_core_test PRIVATE $<TARGET_PROPERTY:vw_core,INCLUDE_DIRECTORIES>)
endif()