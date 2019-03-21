find_package(Boost)
	if (Boost_FOUND)
	add_definitions(-D BOOST_FOUND)
endif()

add_library(vwslim example_predict_builder.cc model_parser.cc opts.cc vw_slim_predict.cc ../../example_predict.cc ../../vw_exception.cc)
install(TARGETS vwslim DESTINATION lib)
