cmake_minimum_required (VERSION 3.7)

# TODO: add filesystem once the windows build works
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

configure_file(config.h.in config.h)

add_library(allreduce allreduce_sockets.cc allreduce_threads.cc vw_exception.cc)
target_include_directories(allreduce PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

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

target_include_directories(vw PUBLIC ${explore_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_compile_features(vw PUBLIC cxx_std_11)
target_compile_definitions(vw PUBLIC _FILE_OFFSET_BITS=64 NDEBUG __extern_always_inline=inline)
set_target_properties(vw PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_features(allreduce PUBLIC cxx_std_11)
target_compile_definitions(allreduce PUBLIC _FILE_OFFSET_BITS=64 NDEBUG __extern_always_inline=inline)
set_target_properties(allreduce PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(vw PUBLIC rapidjson Boost::program_options Threads::Threads zlib allreduce ${CMAKE_DL_LIBS})

# if(MSVC)
#   target_compile_options(vw PRIVATE /W4)
#   target_compile_options(vw PUBLIC /MP -D__SSE2__)

#   target_compile_options(allreduce PRIVATE /W4)
#   target_compile_options(allreduce PUBLIC /MP -D__SSE2__)
# else()
#   target_compile_options(vw PRIVATE -Wall -Wextra -Wpedantic)
#   target_compile_options(vw PUBLIC -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse)

#   target_compile_options(allreduce PRIVATE -Wall -Wextra -Wpedantic)
#   target_compile_options(allreduce PUBLIC -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse)
# endif()

# if(NOT WIN32)
#   add_executable(active_interactor active_interactor.cc)

#   find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)
#   if (HELP2MAN_EXECUTABLE)
#     add_custom_target(manpage ALL
#       COMMAND ${HELP2MAN_EXECUTABLE}
#         --no-info
#         --name="Vowpal Wabbit -- fast online learning tool" $<TARGET_FILE:vw-bin>
#         --output=$<TARGET_FILE:vw-bin>.1
#       DEPENDS vw-bin)
#     INSTALL (FILES $<TARGET_FILE:vw-bin>.1 DESTINATION share/man/man1)
#   endif()
# endif()

# build main executable
add_executable(vw-bin main.cc)
target_link_libraries(vw-bin PRIVATE vw)
set_target_properties(vw-bin PROPERTIES OUTPUT_NAME vw)

if(WIN32)
  add_library(vwdll SHARED vwdll.cpp)
  target_compile_definitions(vw PRIVATE VWDLL_EXPORTS)
  target_link_libraries(vwdll PRIVATE vw)
  set_target_properties(vwdll PROPERTIES OUTPUT_NAME libvw)
endif()

# install(TARGETS vw-bin RUNTIME DESTINATION bin)
# Install_Dependencies("${CMAKE_INSTALL_PREFIX}/bin/vw.exe")
