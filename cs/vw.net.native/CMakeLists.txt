set(vw_net_native_SOURCES
  vw.net.api_status.cc
  vw.net.arguments.cc
  vw.net.builders.cc
  vw.net.cbutil.cc
  vw.net.ds_interaction.cc
  vw.net.example.cc
  vw.net.labels.cc
  vw.net.multi_ex.cc
  vw.net.native.cc
  vw.net.native_exception.cc
  vw.net.predictions.cc
  vw.net.spanning_tree_clr.cc
  vw.net.stream_adapter.cc
  vw.net.workspace.cc
  vw.net.workspace_lda.cc
  vw.net.workspace_parse_json.cc
)

set(vw_net_native_HEADERS
  vw.net.api_status.h
  vw.net.arguments.h
  vw.net.builders.h
  vw.net.cbutil.h
  vw.net.ds_interaction.h
  vw.net.example.h
  vw.net.labels.h
  vw.net.multi_ex.h
  vw.net.native.h
  vw.net.native_exception.h
  vw.net.predictions.h
  vw.net.spanning_tree_clr.h
  vw.net.stream_adapter.h
  vw.net.workspace.h
  vw.net.workspace_lda.h
)

source_group("Sources" FILES ${vw_net_native_SOURCES})
source_group("Headers" FILES ${vw_net_native_HEADERS})

add_library(vw.net.native SHARED
  ${vw_net_native_SOURCES} ${vw_net_native_HEADERS}
)

if(MSVC)
  target_compile_options(vw.net.native PRIVATE /guard:cf)
endif()

set_target_properties(vw.net.native PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(vw.net.native PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  vw.net.native 
  PUBLIC vw_core vw_common vw_config vw_io vw_spanning_tree vw_allreduce
)

# Enable warnings as errors, crossplatform
target_compile_options(vw.net.native PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:
    /WX>
)

install(
  TARGETS vw.net.native
  LIBRARY DESTINATION "./runtimes/${NUGET_RUNTIME_ID}/native/"
  RUNTIME DESTINATION "./runtimes/${NUGET_RUNTIME_ID}/native/"
)
