cmake_minimum_required (VERSION 3.5)
project (vwslim)

set(CMAKE_CXX_STANDARD 11)

enable_testing()

find_package(GTest)
find_package(Threads)
find_library(GMOCK_LIBRARY
	NAMES gmock 
	      libgmock 
	      libgmock.a 
	PATH_SUFFIXES lib)

if (GTEST_FOUND AND Threads_FOUND AND GMOCK_LIBRARY)
	include_directories(${vw_INCLUDE_DIRS})
	include_directories(${GTEST_INCLUDE_DIRS})

	add_definitions(-DVW_NOEXCEPT)

	add_library(${PROJECT_NAME} SHARED ../vowpalwabbit/example_predict.cc example_predict_builder.cc model_parser.cc opts.cc vw_slim_predict.cc)

	if (MSVC)
		target_compile_options(${PROJECT_NAME} PUBLIC /EHs-c-)
	else()
		target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions)
	endif()

	# TODO: this should not run everytime...
	add_custom_command(
		OUTPUT ${PROJECT_SOURCE_DIR}/data/data.h
		COMMAND ./generate-data.sh  $<TARGET_FILE:vw-bin>
		DEPENDS data/generate-data.sh data/cb_data_5.txt
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/data)

	add_executable("${PROJECT_NAME}_test" ut_opts.cpp ut_vw.cpp vw_util.cpp ut_main.cc data/data.h)
	target_link_libraries("${PROJECT_NAME}_test" ${PROJECT_NAME}
		${GTEST_BOTH_LIBRARIES} ${GMOCK_LIBRARY} Threads::Threads)

	# add_test(AllTests ${PROJECT_NAME}_test)
	# GTEST_ADD_TESTS("${PROJECT_NAME}_test" ut_opts.cpp ut_vw.cpp vw_util.cpp ut_main.cc)
endif()
