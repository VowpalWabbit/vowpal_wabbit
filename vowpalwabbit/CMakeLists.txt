configure_file(config.h.in config.h)

# Use position independent code for all targets in this directory
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(allreduce allreduce_sockets.cc allreduce_threads.cc vw_exception.cc)
target_include_directories(allreduce PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(allreduce PUBLIC wsock32 ws2_32)
else()
  target_compile_options(allreduce PUBLIC ${linux_flags})
endif()

set(vw_install_headers
  allreduce.h comp_io.h example.h action_score.h feature_group.h cb_explore.h
  crossplat_compat.h parse_example.h floatbits.h global_data.h io_buf.h learner.h loss_functions.h
  parse_primitives.h parser.h simple_label.h v_array.h vw.h vwdll.h label_parser.h multiclass.h
  cost_sensitive.h cb.h v_hashmap.h memory.h vw_exception.h vw_validate.h multilabel.h constant.h
  ezexample.h
)

set(vw_all_headers
  ${vw_install_headers} accumulate.h options.h options_types.h options_boost_po.h
  options_serializer_boost_po.h correctedMath.h rand48.h log_multi.h recall_tree.h memory_tree.h active_cover.h
  reductions.h active.h cs_active.h lrqfa.h scorer.h csoaa.h lrq.h search_dep_parser.h
  array_parameters_dense.h ect.h marginal.h search_entityrelationtask.h array_parameters.h
  error_reporting.h search_graph.h audit_regressor.h mf.h search.h autolink.h example_predict.h
  search_hooktask.h baseline.h explore_eval.h search_meta.h beam.h expreplay.h multilabel_oaa.h
  search_multiclasstask.h best_constant.h mwt.h search_sequencetask.h bfgs.h network.h sender.h
  binary.h nn.h boosting.h ftrl.h no_label.h spanning_tree.h bs.h gd.h noop.h stable_unique.h
  cache.h gd_mf.h oaa.h stagewise_poly.h cb_adf.h gd_predict.h OjaNewton.h svrg.h cb_algs.h
  gen_cs_example.h parse_args.h topk.h cb_explore_adf.h parse_dispatch_loop.h unique_sort.h
  interact.h interactions.h parse_example_json.h cbify.h warm_cb.h interactions_predict.h vw_allreduce.h
  classweight.h parse_regressor.h kernel_svm.h confidence.h label_dictionary.h
  config.h.in primitives.h lda_core.h print.h vw_versions.h shared_feature_merger.h
)

set(vw_all_sources
  options_boost_po.cc options_serializer_boost_po.cc global_data.cc io_buf.cc
  parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc
  multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc autolink.cc binary.cc lrq.cc
  cost_sensitive.cc multilabel.cc label_dictionary.cc csoaa.cc cb.cc cb_adf.cc cb_algs.cc search.cc
  search_meta.cc search_sequencetask.cc search_dep_parser.cc search_hooktask.cc
  search_multiclasstask.cc search_entityrelationtask.cc search_graph.cc parse_example.cc scorer.cc
  network.cc parse_args.cc accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc
  noop.cc print.cc example.cc parser.cc loss_functions.cc sender.cc nn.cc confidence.cc bs.cc
  cbify.cc warm_cb.cc explore_eval.cc topk.cc stagewise_poly.cc log_multi.cc recall_tree.cc memory_tree.cc active.cc
  active_cover.cc cs_active.cc kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc
  comp_io.cc interactions.cc vw_validate.cc audit_regressor.cc gen_cs_example.cc cb_explore.cc
  action_score.cc cb_explore_adf.cc OjaNewton.cc baseline.cc classweight.cc
  vw_exception.cc no_label.cc shared_feature_merger.cc
)

set(explore_all_headers
  ${explore_INCLUDE_DIRS}explore.h ${explore_INCLUDE_DIRS}explore_internal.h
  ${explore_INCLUDE_DIRS}hash.h
)

# Place the explore headers in their own folder in VS
source_group(explore FILES ${explore_all_headers})

add_library(vw ${vw_all_sources} ${vw_all_headers} ${explore_all_headers})

target_link_libraries(vw PUBLIC rapidjson Boost::program_options ${LINK_THREADS} ZLIB::ZLIB allreduce ${CMAKE_DL_LIBS})

if(GCOV)
  target_link_libraries(vw PUBLIC gcov)
endif()

target_include_directories(vw PUBLIC ${explore_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(vw PUBLIC _FILE_OFFSET_BITS=64 $<$<CONFIG:RELEASE>:NDEBUG>)
# Only define if Clang is not used
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw PUBLIC __extern_always_inline=inline)
endif()

# TODO code analysis
if(WIN32)
  target_compile_definitions(vw PUBLIC __SSE2__)
  target_compile_options(vw PUBLIC /MP)
else()
  target_compile_options(vw PUBLIC ${linux_flags})
endif()

# Turn on warnings
if(WARNINGS)
  if(MSVC)
    target_compile_options(vw PRIVATE /W4)
  else(MSVC)
    target_compile_options(vw PRIVATE -Wall -Wextra -Wpedantic)
  endif(MSVC)
endif(WARNINGS)

if(NOT WIN32)
  add_executable(active_interactor active_interactor.cc)

  if(VW_INSTALL)
    install(TARGETS active_interactor
      RUNTIME DESTINATION bin
    )
  endif()


  target_compile_definitions(active_interactor PUBLIC _FILE_OFFSET_BITS=64)
  target_compile_definitions(active_interactor PUBLIC $<$<CONFIG:RELEASE>:NDEBUG>)
  target_compile_options(active_interactor PUBLIC ${linux_flags})

  find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)
  if (HELP2MAN_EXECUTABLE)
    add_custom_target(manpage ALL
      COMMAND ${HELP2MAN_EXECUTABLE}
        --no-info
        --name="Vowpal Wabbit -- fast online learning tool" $<TARGET_FILE:vw-bin>
        --output=$<TARGET_FILE:vw-bin>.1
      DEPENDS vw-bin)

      if(VW_INSTALL)
        install(FILES $<TARGET_FILE:vw-bin>.1 DESTINATION share/man/man1)
      endif()
  else()
    message(STATUS "help2man not found, please install it to generate manpages")
  endif()
endif()


# build main executable
add_executable(vw-bin main.cc)
target_link_libraries(vw-bin PRIVATE vw)
set_target_properties(vw-bin PROPERTIES OUTPUT_NAME vw)

if(STATIC_LINK_VW)
  target_link_libraries(vw-bin PRIVATE ${unix_static_flag})
endif()

# This is used by Travis to not build vw_c_wrapper, this required <codecvt> which became available in GCC 5.1
if(NOT DEFINED DO_NOT_BUILD_VW_C_WRAPPER)
  if(STATIC_LINK_VW)
    add_library(vw_c_wrapper vwdll.cpp)
  else()
    add_library(vw_c_wrapper SHARED vwdll.cpp)
  endif()

  target_compile_definitions(vw_c_wrapper PUBLIC VWDLL_EXPORTS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE ZLIB_WINAPI)
  target_link_libraries(vw_c_wrapper PUBLIC vw)

  if(VW_INSTALL)
    install(TARGETS vw_c_wrapper
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
    )
  endif()
endif()

if(VW_INSTALL)
  # Install executables
  install(TARGETS vw-bin
  RUNTIME DESTINATION bin
  )

  # Install headers
  install(FILES ${vw_install_headers} ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/vowpalwabbit/vowpalwabbit)
  install(FILES ${explore_all_headers} DESTINATION include/vowpalwabbit/vowpalwabbit/explore)

  # Install libs
  install(TARGETS allreduce vw
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)
endif()
