add_library(allreduce STATIC
    allreduce_sockets.cc allreduce_threads.cc vw_exception.cc
)

target_link_libraries(allreduce Boost::program_options config)

if(MSVC)
    target_link_libraries(allreduce ws2_32)
endif()

add_library(vw
    hash.cc parser_helper.cc global_data.cc io_buf.cc parse_regressor.cc 
    parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc
    multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc
    autolink.cc binary.cc lrq.cc cost_sensitive.cc multilabel.cc
    label_dictionary.cc csoaa.cc cb.cc cb_adf.cc cb_algs.cc search.cc
    search_meta.cc search_sequencetask.cc search_dep_parser.cc
    search_hooktask.cc search_multiclasstask.cc search_entityrelationtask.cc
    search_graph.cc parse_example.cc scorer.cc network.cc parse_args.cc
    accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc
    noop.cc print.cc example.cc parser.cc loss_functions.cc sender.cc nn.cc
    confidence.cc bs.cc cbify.cc explore_eval.cc topk.cc stagewise_poly.cc
    log_multi.cc recall_tree.cc active.cc active_cover.cc cs_active.cc
    kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc
    comp_io.cc interactions.cc vw_exception.cc vw_validate.cc audit_regressor.cc
     gen_cs_example.cc cb_explore.cc action_score.cc cb_explore_adf.cc 
     OjaNewton.cc parse_example_json.cc baseline.cc classweight.cc
)

target_link_libraries(vw
    Boost::program_options ZLIB::ZLIB rapidjson config allreduce
)

if(MSVC)
    target_link_libraries(vw ws2_32)
endif()

target_compile_features(vw PRIVATE
    cxx_right_angle_brackets
    cxx_generalized_initializers
    cxx_lambdas
    cxx_nullptr
)

add_library(vw_c_wrapper
    vwdll.cpp
)

target_link_libraries(vw_c_wrapper vw Boost::program_options config)

add_executable(vowpal_wabbit main.cc)
target_link_libraries(vowpal_wabbit vw)