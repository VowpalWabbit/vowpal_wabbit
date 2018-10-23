configure_file(config.h.in config.h)

# Use position independent code for all targets in this directory
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(allreduce_obj OBJECT allreduce_sockets.cc allreduce_threads.cc vw_exception.cc)
target_include_directories(allreduce_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(allreduce_obj PUBLIC wsock32 ws2_32)
endif()

add_library(allreduce_shared SHARED $<TARGET_OBJECTS:allreduce_obj>)
add_library(allreduce_static STATIC $<TARGET_OBJECTS:allreduce_obj>)

set_target_properties(allreduce_shared PROPERTIES OUTPUT_NAME "allreduce")
set_target_properties(allreduce_static PROPERTIES OUTPUT_NAME "allreduce")

# Expose the allreduce target as static or shared basen on if BUILD_SHARED_LIBS is set.
if(BUILD_SHARED_LIBS)
  add_library(allreduce ALIAS allreduce_shared)
else()
  add_library(allreduce ALIAS allreduce_static)
endif()

set(vw_install_headers
  allreduce.h comp_io.h example.h action_score.h feature_group.h cb_explore.h
  crossplat_compat.h parse_example.h floatbits.h global_data.h io_buf.h learner.h loss_functions.h
  parse_primitives.h parser.h simple_label.h v_array.h vw.h vwdll.h label_parser.h multiclass.h
  cost_sensitive.h cb.h v_hashmap.h memory.h vw_exception.h vw_validate.h multilabel.h constant.h
  ezexample.h
)

set(vw_all_headers
  accumulate.h correctedMath.h learner.h rand48.h action_score.h cost_sensitive.h log_multi.h
  recall_tree.h active_cover.h crossplat_compat.h loss_functions.h reductions.h active.h cs_active.h
  lrqfa.h scorer.h allreduce.h csoaa.h lrq.h search_dep_parser.h array_parameters_dense.h ect.h
  marginal.h search_entityrelationtask.h array_parameters.h error_reporting.h memory.h
  search_graph.h audit_regressor.h example.h mf.h search.h autolink.h example_predict.h multiclass.h
  search_hooktask.h baseline.h explore_eval.h multilabel.h search_meta.h beam.h expreplay.h
  multilabel_oaa.h search_multiclasstask.h best_constant.h ezexample.h mwt.h search_sequencetask.h
  bfgs.h feature_group.h network.h sender.h binary.h floatbits.h nn.h simple_label.h boosting.h
  ftrl.h no_label.h spanning_tree.h bs.h gd.h noop.h stable_unique.h cache.h gd_mf.h oaa.h
  stagewise_poly.h cb_adf.h gd_predict.h OjaNewton.h svrg.h cb_algs.h gen_cs_example.h parse_args.h
  topk.h cb_explore_adf.h global_data.h parse_dispatch_loop.h unique_sort.h cb_explore.h interact.h
  parse_example.h v_array.h cb.h interactions.h parse_example_json.h v_hashmap.h cbify.h
  interactions_predict.h parse_primitives.h vw_allreduce.h classweight.h io_buf.h parse_regressor.h
  vwdll.h comp_io.h kernel_svm.h parser.h vw_exception.h confidence.h label_dictionary.h
  parser_helper.h vw.h config.h.in label_parser.h primitives.h vw_validate.h constant.h lda_core.h
  print.h vw_versions.h
)

set(vw_all_sources
  global_data.cc io_buf.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc
  simple_label.cc multiclass.cc oaa.cc multilabel_oaa.cc boosting.cc ect.cc marginal.cc autolink.cc
  binary.cc lrq.cc cost_sensitive.cc multilabel.cc label_dictionary.cc csoaa.cc cb.cc cb_adf.cc
  cb_algs.cc search.cc search_meta.cc search_sequencetask.cc search_dep_parser.cc search_hooktask.cc
  search_multiclasstask.cc search_entityrelationtask.cc search_graph.cc parse_example.cc scorer.cc
  network.cc parse_args.cc accumulate.cc gd.cc learner.cc mwt.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc
  noop.cc print.cc example.cc parser.cc loss_functions.cc sender.cc nn.cc confidence.cc bs.cc
  cbify.cc explore_eval.cc topk.cc stagewise_poly.cc log_multi.cc recall_tree.cc active.cc
  active_cover.cc cs_active.cc kernel_svm.cc best_constant.cc ftrl.cc svrg.cc lrqfa.cc interact.cc
  comp_io.cc interactions.cc vw_validate.cc audit_regressor.cc gen_cs_example.cc cb_explore.cc
  action_score.cc cb_explore_adf.cc OjaNewton.cc parse_example_json.cc baseline.cc classweight.cc
  vw_exception.cc parser_helper.cc no_label.cc
)

set(explore_all_headers
  ${explore_INCLUDE_DIRS}explore.h ${explore_INCLUDE_DIRS}explore_internal.h
  ${explore_INCLUDE_DIRS}hash.h
)

# Place the explore headers in their own folder in VS
source_group(explore ${explore_all_headers})
add_library(vw_obj OBJECT ${vw_all_sources} ${vw_all_headers} ${explore_all_headers})

# Create both shared and static libs
add_library(vw_shared SHARED $<TARGET_OBJECTS:vw_obj>)
add_library(vw_static STATIC $<TARGET_OBJECTS:vw_obj>)

target_link_libraries(vw_shared PUBLIC vw_obj)
target_link_libraries(vw_static PUBLIC vw_obj)

set_target_properties(vw_shared PROPERTIES OUTPUT_NAME "vw")
set_target_properties(vw_static PROPERTIES OUTPUT_NAME "vw")

# Expose the vw target as static or shared basen on if BUILD_SHARED_LIBS is set.
if(BUILD_SHARED_LIBS)
  add_library(vw ALIAS vw_shared)
else()
  add_library(vw ALIAS vw_static)
endif()

# Add -ffast-math for speed, remove for testability.
set(linux_standard_config -O3 -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse)
# Valgrind / gdb debugging
set(linux_debug_config -g -O0)

# Set base flags per configuration
set(linux_flags $<$<CONFIG:DEBUG>:${linux_debug_config}> $<$<CONFIG:RELEASE>:${linux_standard_config}>)

if(PROFILE)
  # for profiling -- note that it needs to be gcc
  set(linux_flags ${linux_flags} -fno-strict-aliasing -pg)
endif()

if(VALGRIND_PROFILE)
  # for valgrind profiling: run 'valgrind --tool=callgrind PROGRAM' then 'callgrind_annotate --tree=both --inclusive=yes'
  set(linux_flags ${linux_flags} -ffast-math -g -fomit-frame-pointer -fno-strict-aliasing)
endif()

if(GCOV)
  # gcov configuration
  set(linux_flags ${linux_flags} -g -O0 -fprofile-arcs -ftest-coverage -fno-strict-aliasing -pg -lgcov)
endif()

target_link_libraries(vw_obj PUBLIC rapidjson Boost::program_options Threads::Threads ZLIB::ZLIB allreduce ${CMAKE_DL_LIBS})
target_include_directories(vw_obj PUBLIC ${explore_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Change the below varible to choose a different debug config.
set(linux_selected_debug_config ${linux_debug_config})

target_compile_definitions(vw_obj PUBLIC _FILE_OFFSET_BITS=64 $<$<CONFIG:RELEASE>:NDEBUG>)
# Only define if Clang is not used
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_definitions(vw_obj PUBLIC __extern_always_inline=inline)
endif()

# TODO code analysis
if(WIN32)
  target_compile_definitions(vw_obj PUBLIC __SSE2__)
  target_compile_options(vw_obj PUBLIC /MP)

  target_compile_options(vw_obj PUBLIC $<$<CONFIG:DEBUG>:/INCREMENTAL /Od /Zi /analyze->)
  target_compile_options(vw_obj PUBLIC $<$<CONFIG:RELEASE>:/GL /O2 /Gy /Oi /Ob2 /Ot /Oy /GT /MD>)
else()
  target_compile_options(vw_obj PUBLIC ${linux_flags})
endif()

# Turn on warnings
if(WARNINGS)
  if(MSVC)
    target_compile_options(vw_obj PRIVATE /W4)
  else(MSVC)
    target_compile_options(vw_obj PRIVATE -Wall -Wextra -Wpedantic)
  endif(MSVC)
endif(WARNINGS)

if(NOT WIN32)
  add_executable(active_interactor active_interactor.cc)

  install(TARGETS active_interactor
    RUNTIME DESTINATION bin
  )

  target_compile_definitions(active_interactor PUBLIC _FILE_OFFSET_BITS=64)
  target_compile_definitions(active_interactor PUBLIC $<$<CONFIG:RELEASE>:NDEBUG>)
  target_compile_options(active_interactor PUBLIC ${linux_standard_config})

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
# TODO https://stackoverflow.com/questions/24648357/compiling-a-static-executable-with-cmake
# Static exe
target_link_libraries(vw-bin PRIVATE vw)
set_target_properties(vw-bin PROPERTIES OUTPUT_NAME vw)

add_library(vw_c_wrapper_obj OBJECT vwdll.cpp)
target_compile_definitions(vw_c_wrapper_obj PUBLIC VWDLL_EXPORTS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE ZLIB_WINAPI)
target_link_libraries(vw_c_wrapper_obj PUBLIC vw)

add_library(vw_c_wrapper_shared SHARED $<TARGET_OBJECTS:vw_c_wrapper_obj>)
add_library(vw_c_wrapper_static STATIC $<TARGET_OBJECTS:vw_c_wrapper_obj>)

set_target_properties(vw_c_wrapper_shared PROPERTIES OUTPUT_NAME "vw_c_wrapper")
set_target_properties(vw_c_wrapper_static PROPERTIES OUTPUT_NAME "vw_c_wrapper")

# Expose the vw_c_wrapper target as static or shared basen on if BUILD_SHARED_LIBS is set.
if(BUILD_SHARED_LIBS)
  add_library(vw_c_wrapper ALIAS vw_shared)
else()
  add_library(vw_c_wrapper ALIAS vw_static)
endif()


# Install executables
install(TARGETS vw-bin
  RUNTIME DESTINATION bin
)

# Install headers
install(FILES ${vw_install_headers} ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/vowpalwabbit/vowpalwabbit)
install(FILES ${explore_all_headers} DESTINATION include/vowpalwabbit/vowpalwabbit/explore)

# Install libs
install(TARGETS allreduce_shared allreduce_static vw_shared vw_static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(TARGETS vw_c_wrapper_shared vw_c_wrapper_static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)
