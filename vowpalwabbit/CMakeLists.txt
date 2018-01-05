cmake_minimum_required (VERSION 3.5)
project (vw)

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
	# /MP multi processor
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64 /MP")

	# if(VW_SSE2)
	#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	# endif()
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64 -fomit-frame-pointer -fno-strict-aliasing -msse2")

	#if(VW_SSE2)
	#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	#endif()
endif()

# TODO: add filesystem once the windows build works
find_package(Boost ${Boost_VW_VERSION} REQUIRED COMPONENTS program_options)
find_package(RapidJSON REQUIRED)
find_package(Threads REQUIRED)  
find_package(ZLIB REQUIRED)

include_directories(${Boost_INCLUDE_DIRS}) 
include_directories(${rapidjson_INCLUDE_DIRS}) 
include_directories(${vwcommon_INCLUDE_DIRS})
include_directories(${allreduce_INCLUDE_DIRS})

configure_file(config.h.in config.h)

add_library(vw hash.cc global_data.cc io_buf.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc autolink.cc binary.cc lrq.cc cost_sensitive.cc multilabel.cc label_dictionary.cc csoaa.cc cb.cc cb_adf.cc cb_algs.cc search.cc search_meta.cc search_sequencetask.cc search_dep_parser.cc search_hooktask.cc search_multiclasstask.cc search_entityrelationtask.cc search_graph.cc parse_example.cc scorer.cc network.cc parse_args.cc accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc noop.cc print.cc example.cc parser.cc loss_functions.cc sender.cc nn.cc confidence.cc bs.cc cbify.cc explore_eval.cc topk.cc stagewise_poly.cc log_multi.cc recall_tree.cc active.cc active_cover.cc cs_active.cc kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc comp_io.cc interactions.cc vw_validate.cc audit_regressor.cc gen_cs_example.cc cb_explore.cc action_score.cc cb_explore_adf.cc OjaNewton.cc parse_example_json.cc baseline.cc classweight.cc)

target_link_libraries(vw PUBLIC allreduce vwcommon ${Boost_LIBRARIES} Threads::Threads ${ZLIB_LIBRARIES})

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} ${vwcommon_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

# build main executable
add_executable(vw-bin main.cc) 
target_link_libraries(vw-bin PRIVATE vw)
SET_TARGET_PROPERTIES(vw-bin PROPERTIES OUTPUT_NAME vw)
install(TARGETS vw-bin RUNTIME DESTINATION bin)
