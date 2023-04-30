if(VW_INSTALL)
    message(FATAL_ERROR "Install not supported for WASM build" )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/out/")

add_executable(vw-wasm wasm_wrapper.cc)
set_target_properties(vw-wasm PROPERTIES LINK_FLAGS "-fexceptions -s WASM=1 -s NO_DYNAMIC_EXECUTION=1 --bind -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS=\"['_malloc', '_free']\"")
target_link_libraries(vw-wasm PUBLIC vw_explore vw_core vw_explore "-fexceptions")
