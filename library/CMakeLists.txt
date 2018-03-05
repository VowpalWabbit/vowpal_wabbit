if(BUILD_TESTING)
	add_executable(library_example library_example.cc)
	target_link_libraries(library_example vw)
	target_compile_features(library_example PRIVATE cxx_generalized_initializers)

	add_executable(ezexample_predict ezexample_predict.cc)
	target_compile_features(ezexample_predict PRIVATE cxx_generalized_initializers)
	target_link_libraries(ezexample_predict vw)

	add_executable(ezexample_train ezexample_train.cc)
	target_compile_features(ezexample_train PRIVATE cxx_generalized_initializers)
	target_link_libraries(ezexample_train vw)

#	add_executable(recommend recommend.cc)
#	target_compile_features(recommend PRIVATE cxx_generalized_initializers)
#	target_link_libraries(recommend vw)

#	add_executable(gd_mf_weights gd_mf_weights.cc)
#	target_compile_features(gd_mf_weights PRIVATE cxx_generalized_initializers)
#	target_link_libraries(gd_mf_weights vw)
#
#	add_executable(test_search test_search.cc)
#	target_compile_features(test_search PRIVATE cxx_generalized_initializers)
#	target_link_libraries(test_search vw)
endif()