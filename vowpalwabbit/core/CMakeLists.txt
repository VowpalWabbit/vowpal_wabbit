set(vw_core_headers
  include/vw/core/accumulate.h
  include/vw/core/action_score.h
  include/vw/core/active_multiclass_prediction.h
  include/vw/core/api_status.h
  include/vw/core/array_parameters_dense.h
  include/vw/core/array_parameters.h
  include/vw/core/beam.h
  include/vw/core/best_constant.h
  include/vw/core/cache.h
  include/vw/core/cached_learner.h
  include/vw/core/cb_continuous_label.h
  include/vw/core/cb_label_parser.h
  include/vw/core/cb_type.h
  include/vw/core/cb.h
  include/vw/core/ccb_label.h
  include/vw/core/ccb_reduction_features.h
  include/vw/core/compat.h
  include/vw/core/constant.h
  include/vw/core/continuous_actions_reduction_features.h
  include/vw/core/correctedMath.h
  include/vw/core/cost_sensitive.h
  include/vw/core/crossplat_compat.h
  include/vw/core/debug_log.h
  include/vw/core/debug_print.h
  include/vw/core/decision_scores.h
  include/vw/core/distributionally_robust.h
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
  include/vw/core/generated_interactions_reduction_features.h
  include/vw/core/generic_range.h
  include/vw/core/global_data.h
  include/vw/core/guard.h
  include/vw/core/hashstring.h
  include/vw/core/interactions_predict.h
  include/vw/core/io_buf.h
  include/vw/core/json_utils.h
  include/vw/core/kskip_ngram_transformer.h
  include/vw/core/label_dictionary.h
  include/vw/core/label_parser.h
  include/vw/core/label_type.h
  include/vw/core/learner.h
  include/vw/core/loss_functions.h
  include/vw/core/memory.h
  include/vw/core/metric_sink.h
  include/vw/core/model_utils.h
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
  include/vw/core/qr_decomposition.h
  include/vw/core/queue.h
  include/vw/core/rand_state.h
  include/vw/core/rand48.h
  include/vw/core/reduction_features.h
  include/vw/core/reduction_stack.h
  include/vw/core/reductions_fwd.h
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
  include/vw/core/interactions.h
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
  include/vw/core/scored_config.h
  include/vw/core/shared_data.h
  include/vw/core/simple_label_parser.h
  include/vw/core/simple_label.h
  include/vw/core/slates_label.h
  include/vw/core/stable_unique.h
  include/vw/core/tag_utils.h
  include/vw/core/text_utils.h
  include/vw/core/unique_sort.h
  include/vw/core/v_array.h
  include/vw/core/version.h
  include/vw/core/vw_allreduce.h
  include/vw/core/vw_math.h
  include/vw/core/vw_validate.h
  include/vw/core/vw_versions.h
  include/vw/core/vw.h
)

set(vw_core_sources
  src/accumulate.cc
  src/action_score.cc
  src/api_status.cc
  src/best_constant.cc
  src/cache.cc
  src/cb_continuous_label.cc
  src/cb_type.cc
  src/cb.cc
  src/ccb_label.cc
  src/ccb_reduction_features.cc
  src/cost_sensitive.cc
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
  src/io_buf.cc
  src/kskip_ngram_transformer.cc
  src/label_dictionary.cc
  src/label_parser.cc
  src/label_type.cc
  src/learner.cc
  src/loss_functions.cc
  src/metric_sink.cc
  src/multiclass.cc
  src/multilabel.cc
  src/named_labels.cc
  src/network.cc
  src/no_label.cc
  src/parse_args.cc
  src/parse_example.cc
  src/parse_primitives.cc
  src/parse_regressor.cc
  src/parser.cc
  src/prediction_type.cc
  src/print_utils.cc
  src/prob_dist_cont.cc
  src/qr_decomposition.cc
  src/rand48.cc
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
  src/reductions/ect.cc
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
  src/interactions.cc
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
  src/scored_config.cc
  src/shared_data.cc
  src/simple_label_parser.cc
  src/simple_label.cc
  src/slates_label.cc
  src/tag_utils.cc
  src/text_utils.cc
  src/unique_sort.cc
  src/version.cc
  src/vw_validate.cc
)

# Installation is disabled for external parser because it would break the contract of include dirs and this project cant enforce it.
# TODO: Remove external parser as an internal concept so that external parser just uses vw as a lib.
set(ENABLE_INSTALL_OPTION "ENABLE_INSTALL")
if(BUILD_EXTERNAL_PARSER)
  set(vw_all_sources ${vw_all_sources} ${external_parser_sources})
  set(ENABLE_INSTALL_OPTION "")
endif()

if(BUILD_EXTERNAL_PARSER)
  set(vw_core_sources ${vw_core_sources} ${external_parser_headers})
endif()

vw_add_library(
    NAME "core"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_core_headers} ${vw_core_sources}
    PUBLIC_DEPS vw_common vw_explore vw_allreduce vw_config ${spdlog_target} fmt::fmt
    PRIVATE_DEPS
        ${CMAKE_DL_LIBS} ${LINK_THREADS} vw_io $<BUILD_INTERFACE:${boost_math_target}> $<BUILD_INTERFACE:eigen>
        # Workaround an issue where RapidJSON needed to be exported tom install the target. This is
        # actually a private dependency and so do not "link" when processing targets for installation.
        # https://gitlab.kitware.com/cmake/cmake/issues/15415
        $<BUILD_INTERFACE:RapidJSON>
    DESCRIPTION "This contains all remaining VW code, all reduction implementations, driver, option handling"
    EXCEPTION_DESCRIPTION "Yes"
    ${ENABLE_INSTALL_OPTION}
)

target_include_directories(vw_core PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
if (BUILD_EXTERNAL_PARSER)
  target_compile_definitions(vw_core PUBLIC BUILD_EXTERNAL_PARSER)
  target_include_directories(vw_core PRIVATE ${EXTERNAL_PARSER_DIR})
  target_link_libraries(vw_core PRIVATE ${EXTERNAL_PARSER_LIBS})
endif()

if(BUILD_FLATBUFFERS)
  target_link_libraries(vw_core PRIVATE vw_fb_parser)
  target_compile_definitions(vw_core PUBLIC BUILD_FLATBUFFERS)
endif()

if(BUILD_PRIVACY_ACTIVATION)
  target_compile_definitions(vw_core PUBLIC PRIVACY_ACTIVATION)
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
    SOURCES tests/cache_test.cc tests/parse_args_test.cc
)
