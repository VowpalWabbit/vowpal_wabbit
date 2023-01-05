set(vw_c_public_headers
  include/vw/experimental/options.h
  include/vw/experimental/placeholder.h
  include/vw/experimental/workspace.h
  include/vw/experimental/types.h
)

set(vw_c_private_headers
  src/c_io_adapter.h
)

set(vw_c_impl_files
  src/options.cc
  src/placeholder.cc
  src/c_io_adapter.cc
  src/workspace.cc
)

# In order to reuse the error definition data in the C API we must make the error_data.h header available here.
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../vowpalwabbit/core/include/vw/core/error_data.h include/vw/experimental/error_data.h COPYONLY)
# Reuse visibility macros
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../vowpalwabbit/common/include/vw/common/vwvis.h include/vw/experimental/vwvis.h COPYONLY)

add_library(vw_c_api ${vw_c_public_headers} ${vw_c_impl_files} ${vw_c_private_headers})
target_include_directories(vw_c_api PUBLIC include/ ${CMAKE_CURRENT_BINARY_DIR}/include/)
target_link_libraries(vw_c_api PRIVATE vw_core vw_io)
target_compile_definitions(vw_c_api PRIVATE VWDLL_EXPORTS)