if(VW_INSTALL)
    message(FATAL_ERROR "Install not supported for WASM build" )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/dist/")

add_executable(vw-wasm src/wasm_wrapper.cc)
set_target_properties(vw-wasm PROPERTIES LINK_FLAGS "-fexceptions -s WASM=1 -s SINGLE_FILE=1 -s NO_DYNAMIC_EXECUTION=1 --bind -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -s EXPORTED_FUNCTIONS=\"['_malloc', '_free']\" -s EXPORTED_RUNTIME_METHODS=\"['HEAPU8']\" -s MODULARIZE=1 -s EXPORT_NAME=\"vwWasmModule\"")
target_link_libraries(vw-wasm PUBLIC vw_explore vw_core "-fexceptions")