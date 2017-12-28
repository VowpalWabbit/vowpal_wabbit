cmake_minimum_required (VERSION 3.5)
project (VowpalWabbit)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

set (CMAKE_CXX_STANDARD 14)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
set(THREADS_PREFER_PTHREAD_FLAG ON)
#set(OPENSSL_USE_STATIC_LIBS TRUE)

# DSVM only has Boost 1.58.0 installed
find_package(Boost 1.58.0 COMPONENTS program_options filesystem python regex)
find_package(RapidJSON REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIRS}) 
include_directories(${rapidjson_INCLUDE_DIRS}) 
include_directories(${cpprest_INCLUDE_DIRS}) 

add_library(ds hash.cc global_data.cc io_buf.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc autolink.cc binary.cc lrq.cc cost_sensitive.cc multilabel.cc label_dictionary.cc csoaa.cc cb.cc cb_adf.cc cb_algs.cc search.cc search_meta.cc search_sequencetask.cc search_dep_parser.cc search_hooktask.cc search_multiclasstask.cc search_entityrelationtask.cc search_graph.cc parse_example.cc scorer.cc network.cc parse_args.cc accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc noop.cc print.cc example.cc parser.cc loss_functions.cc sender.cc nn.cc confidence.cc bs.cc cbify.cc explore_eval.cc topk.cc stagewise_poly.cc log_multi.cc recall_tree.cc active.cc active_cover.cc cs_active.cc kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc comp_io.cc interactions.cc vw_exception.cc vw_validate.cc audit_regressor.cc gen_cs_example.cc cb_explore.cc action_score.cc cb_explore_adf.cc OjaNewton.cc parse_example_json.cc baseline.cc classweight.cc ds_api.cc ds_vw.cc allreduce_sockets.cc allreduce_threads.cc vw_exception.cc event_hub_client.cc)

add_executable(vw main.cc hash.cc global_data.cc io_buf.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc autolink.cc binary.cc lrq.cc cost_sensitive.cc multilabel.cc label_dictionary.cc csoaa.cc cb.cc cb_adf.cc cb_algs.cc search.cc search_meta.cc search_sequencetask.cc search_dep_parser.cc search_hooktask.cc search_multiclasstask.cc search_entityrelationtask.cc search_graph.cc parse_example.cc scorer.cc network.cc parse_args.cc accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc noop.cc print.cc example.cc parser.cc loss_functions.cc sender.cc nn.cc confidence.cc bs.cc cbify.cc explore_eval.cc topk.cc stagewise_poly.cc log_multi.cc recall_tree.cc active.cc active_cover.cc cs_active.cc kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc comp_io.cc interactions.cc vw_exception.cc vw_validate.cc audit_regressor.cc gen_cs_example.cc cb_explore.cc action_score.cc cb_explore_adf.cc OjaNewton.cc parse_example_json.cc baseline.cc classweight.cc allreduce_sockets.cc allreduce_threads.cc vw_exception.cc)

target_link_libraries(ds PRIVATE ${Boost_LIBRARIES} ${cpprest_LIBRARY} Threads::Threads ${ZLIB_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
# target_link_libraries(ds PRIVATE ${Boost_LIBRARIES} ${cpprest_LIBRARY} Threads::Threads ${ZLIB_LIBRARIES} ${OpenSSL_LIBRARIES})
target_link_libraries(vw PRIVATE ${Boost_LIBRARIES} Threads::Threads ${ZLIB_LIBRARIES})

install(TARGETS ds vw
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include)
