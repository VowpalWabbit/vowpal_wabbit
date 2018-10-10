configure_file(config.h.in config.h)

add_library(allreduce allreduce_sockets.cc allreduce_threads.cc vw_exception.cc)
target_include_directories(allreduce PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(allreduce PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(allreduce wsock32 ws2_32)
endif()

add_library(vw_common INTERFACE)
target_compile_definitions(vw_common INTERFACE _FILE_OFFSET_BITS=64)
# Only define if Clang is used
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw_common INTERFACE __extern_always_inline=inline)
endif()

# Turn on warnings
# These warnings are too much to have on by default until they are fixed
# if(MSVC)
#   target_compile_options(vw_common INTERFACE /W4)
# else()
#   target_compile_options(vw_common INTERFACE -Wall -Wextra -Wpedantic)
# endif()

target_include_directories(vw_common INTERFACE ${explore_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(vw_common INTERFACE rapidjson Boost::program_options Threads::Threads zlib allreduce ${CMAKE_DL_LIBS})

add_library(standard_config INTERFACE)
target_compile_definitions(standard_config INTERFACE NDEBUG)

# Add -ffast-math for speed, remove for testability.
target_compile_options(standard_config INTERFACE -O3 -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse)

# for profiling -- note that it needs to be gcc
add_library(profile_config INTERFACE)
target_compile_options(profile_config INTERFACE -fno-strict-aliasing -pg)

# Valgrind / gdb debugging
add_library(debug_config INTERFACE)
target_compile_options(debug_config INTERFACE  -g -O0)

# for valgrind profiling: run 'valgrind --tool=callgrind PROGRAM' then 'callgrind_annotate --tree=both --inclusive=yes'
add_library(valgrind_profile_config INTERFACE)
target_compile_options(valgrind_profile_config INTERFACE -ffast-math -g -fomit-frame-pointer -fno-strict-aliasing)

# gcov configuration
add_library(gcov_config INTERFACE)
target_compile_options(profile_config INTERFACE -g -O0 -fprofile-arcs -ftest-coverage -fno-strict-aliasing -pg -lgcov)

set(vw_sources
  global_data.cc io_buf.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc
  multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc autolink.cc binary.cc lrq.cc cost_sensitive.cc multilabel.cc
  label_dictionary.cc csoaa.cc cb.cc cb_adf.cc cb_algs.cc search.cc search_meta.cc search_sequencetask.cc search_dep_parser.cc
  search_hooktask.cc search_multiclasstask.cc search_entityrelationtask.cc search_graph.cc parse_example.cc scorer.cc network.cc
  parse_args.cc accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc noop.cc print.cc example.cc parser.cc
  loss_functions.cc sender.cc nn.cc confidence.cc bs.cc cbify.cc explore_eval.cc topk.cc stagewise_poly.cc log_multi.cc
  recall_tree.cc active.cc active_cover.cc cs_active.cc kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc
  comp_io.cc interactions.cc vw_validate.cc audit_regressor.cc gen_cs_example.cc cb_explore.cc action_score.cc cb_explore_adf.cc
  OjaNewton.cc parse_example_json.cc baseline.cc classweight.cc vw_exception.cc parser_helper.cc no_label.cc
)

# Replace standard_config with one of the following options: standard_config, profile_config, debug_config, valgrind_profile_config, gcov_config
add_library(vw ${vw_sources})
target_link_libraries(vw PUBLIC vw_common standard_config)
set_target_properties(vw PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(NOT WIN32)
  add_executable(active_interactor active_interactor.cc)
  target_link_libraries(active_interactor PUBLIC vw_common standard_config)
  set_target_properties(active_interactor PROPERTIES POSITION_INDEPENDENT_CODE ON)
  install(TARGETS active_interactor
    RUNTIME DESTINATION bin
  )

  find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)
  if (HELP2MAN_EXECUTABLE)
    add_custom_target(manpage ALL
      COMMAND ${HELP2MAN_EXECUTABLE}
        --no-info
        --name="Vowpal Wabbit -- fast online learning tool" $<TARGET_FILE:vw-bin>
        --output=$<TARGET_FILE:vw-bin>.1
      DEPENDS vw-bin)
    install(FILES $<TARGET_FILE:vw-bin>.1 DESTINATION share/man/man1)
  else()
    message(STATUS "help2man not found, please install it to generate manpages")
  endif()
endif()


# build main executable
add_executable(vw-bin main.cc)
target_link_libraries(vw-bin PRIVATE vw)
set_target_properties(vw-bin PROPERTIES OUTPUT_NAME vw)

if(WIN32)
  add_library(vwdll SHARED vwdll.cpp)
  target_compile_definitions(vw PRIVATE VWDLL_EXPORTS)
  target_link_libraries(vwdll PRIVATE vw)
  set_target_properties(vwdll PROPERTIES OUTPUT_NAME libvw)

  install(TARGETS vwdll
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
endif()

# TODO do other dependencies need to be installed here too?
# TODO install libs and includes
install(TARGETS vw-bin active_interactor
  RUNTIME DESTINATION bin
)
