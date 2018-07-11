cmake_minimum_required (VERSION 3.5)
project (vw)

set(CMAKE_CXX_STANDARD 11)

if(WIN32)
	# /MP multi processor
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64 /MP -D__SSE2__")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64 -fomit-frame-pointer -fno-strict-aliasing -msse2")
endif()

# TODO: add filesystem once the windows build works
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(RapidJSON REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

# ==================
# Hunt for features:
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(xmmintrin.h HAVE_XMMINTRIN_H)

include_directories(${explore_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${RAPIDJSON_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(config.h.in config.h)

add_library(allreduce allreduce_sockets.cc allreduce_threads.cc vw_exception.cc)
target_link_libraries(allreduce ${Boost_LIBRARIES})

if(WIN32)
    find_library(WS2_32_LIBRARY ws2_32)
    target_link_libraries(allreduce ${WS2_32_LIBRARY})
endif()

add_library(vw global_data.cc io_buf.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc 
	multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc autolink.cc binary.cc lrq.cc cost_sensitive.cc multilabel.cc
	label_dictionary.cc csoaa.cc cb.cc cb_adf.cc cb_algs.cc search.cc search_meta.cc search_sequencetask.cc search_dep_parser.cc 
	search_hooktask.cc search_multiclasstask.cc search_entityrelationtask.cc search_graph.cc parse_example.cc scorer.cc network.cc
	parse_args.cc accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc noop.cc print.cc example.cc parser.cc
	loss_functions.cc sender.cc nn.cc confidence.cc bs.cc cbify.cc explore_eval.cc topk.cc stagewise_poly.cc log_multi.cc 
	recall_tree.cc active.cc active_cover.cc cs_active.cc kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc 
	comp_io.cc interactions.cc vw_validate.cc audit_regressor.cc gen_cs_example.cc cb_explore.cc action_score.cc cb_explore_adf.cc 
	OjaNewton.cc parse_example_json.cc baseline.cc classweight.cc vw_exception.cc parser_helper.cc no_label.cc)

target_link_libraries(vw PUBLIC ${Boost_LIBRARIES} Threads::Threads ${ZLIB_LIBRARIES} allreduce)

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${explore_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

if(NOT WIN32)
	add_executable(active_interactor active_interactor.cc)

	find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)

	if (HELP2MAN_EXECUTABLE)
		add_custom_target(manpage ALL
			COMMAND ${HELP2MAN_EXECUTABLE} 
				--no-info 
				--name="Vowpal Wabbit -- fast online learning tool" $<TARGET_FILE:vw-bin>
				--output=$<TARGET_FILE:vw-bin>.1
			DEPENDS vw-bin)

		INSTALL (FILES $<TARGET_FILE:vw-bin>.1 DESTINATION share/man/man1)
	endif()
endif()

# build main executable
add_executable(vw-bin main.cc) 
target_link_libraries(vw-bin PRIVATE vw)
SET_TARGET_PROPERTIES(vw-bin PROPERTIES OUTPUT_NAME vw)

if(WIN32)
	add_library(vwdll SHARED vwdll.cpp)
	set_target_properties (vwdll PROPERTIES COMPILE_FLAGS "-DVWDLL_EXPORTS")
	target_link_libraries(vwdll PRIVATE vw)
	SET_TARGET_PROPERTIES(vwdll PROPERTIES OUTPUT_NAME libvw)
endif()

install(TARGETS vw-bin RUNTIME DESTINATION bin)
Install_Dependencies("${CMAKE_INSTALL_PREFIX}/bin/vw.exe")
