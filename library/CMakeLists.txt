add_executable(ezexample_predict ezexample_predict.cc)
target_link_libraries(ezexample_predict PRIVATE vw)

add_executable(ezexample_predict_threaded ezexample_predict_threaded.cc)
target_link_libraries(ezexample_predict_threaded PRIVATE vw Boost::system Boost::thread)

add_executable(ezexample_train ezexample_train.cc)
target_link_libraries(ezexample_train PRIVATE vw)

add_executable(library_example library_example.cc)
target_link_libraries(library_example PRIVATE vw)

add_executable(test_search test_search.cc)
target_link_libraries(test_search PRIVATE vw)

add_executable(search_generate search_generate.cc)
target_link_libraries(search_generate PRIVATE vw)

if(NOT WIN32)
  add_executable(recommend recommend.cc)
  target_link_libraries(recommend PRIVATE vw)

  add_executable(gd_mf_weights gd_mf_weights.cc)
  target_link_libraries(gd_mf_weights PRIVATE vw)
endif()

add_custom_target(all_library_examples DEPENDS ezexample_predict ezexample_predict_threaded ezexample_train library_example test_search search_generate recommend gd_mf_weights)
