configure_file(config.h.in config.h)

# Add -ffast-math for speed, remove for testability.
set(linux_standard_config -O3 -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse)
# for profiling -- note that it needs to be gcc
set(linux_profile_config -fno-strict-aliasing -pg)
# Valgrind / gdb debugging
set(linux_debug_config -g -O0)
# for valgrind profiling: run 'valgrind --tool=callgrind PROGRAM' then 'callgrind_annotate --tree=both --inclusive=yes'
set(linux_valgrind_profile_config -ffast-math -g -fomit-frame-pointer -fno-strict-aliasing)
# gcov configuration
set(linux_profile_config -g -O0 -fprofile-arcs -ftest-coverage -fno-strict-aliasing -pg -lgcov)

add_library(allreduce allreduce_sockets.cc allreduce_threads.cc vw_exception.cc)
target_include_directories(allreduce PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(allreduce PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(allreduce wsock32 ws2_32)
endif()

add_library(vw
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

target_link_libraries(vw PUBLIC rapidjson Boost::program_options Threads::Threads ZLIB::ZLIB allreduce ${CMAKE_DL_LIBS})
target_include_directories(vw PUBLIC ${explore_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(vw PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Change the below varible to choose a different debug config.
set(linux_selected_debug_config ${linux_debug_config})

target_compile_definitions(vw PUBLIC _FILE_OFFSET_BITS=64)
# Only define if Clang is not used
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw PUBLIC __extern_always_inline=inline)
endif()

# TODO code analysis
if(WIN32)
  target_compile_definitions(vw PUBLIC __SSE2__)
  target_compile_options(vw PUBLIC /MP)

  target_compile_options(vw PUBLIC $<$<CONFIG:DEBUG>:/INCREMENTAL /Od /Zi /analyze->)
  target_compile_options(vw PUBLIC $<$<CONFIG:RELEASE>:/GL /O2 /Gy /Oi /Ob2 /Ot /Oy /GT /MD>)
else()
  target_compile_definitions(vw PUBLIC $<$<CONFIG:RELEASE>:NDEBUG>)
  target_compile_options(vw PUBLIC $<$<CONFIG:DEBUG>:${linux_selected_debug_config}>)
  target_compile_options(vw PUBLIC $<$<CONFIG:RELEASE>:${linux_standard_config}>)
endif()

# Turn on warnings
# These warnings are too much to have on by default until they are fixed
# if(MSVC)
#   target_compile_options(vw PRIVATE /W4)
# else()
#   target_compile_options(vw PRIVATE -Wall -Wextra -Wpedantic)
# endif()

if(NOT WIN32)
  add_executable(active_interactor active_interactor.cc)

  target_compile_definitions(active_interactor PUBLIC _FILE_OFFSET_BITS=64)
  target_compile_definitions(active_interactor PUBLIC $<$<CONFIG:RELEASE>:NDEBUG>)
  target_compile_options(active_interactor PUBLIC ${linux_standard_config})

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
  target_compile_definitions(vwdll PRIVATE VWDLL_EXPORTS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE ZLIB_WINAPI)
  target_compile_options(vwdll PRIVATE /utf-8)
  target_link_libraries(vwdll PRIVATE vw)
  set_target_properties(vwdll PROPERTIES OUTPUT_NAME libvw)

  install(TARGETS vwdll
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
endif()

# TODO do other dependencies need to be installed here too?
# TODO install libs and includes
install(TARGETS vw-bin
  RUNTIME DESTINATION bin
)

