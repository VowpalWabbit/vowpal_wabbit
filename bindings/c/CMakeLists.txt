set(vw_c_public_headers
  include/vw/experimental/placeholder.h
)

set(vw_c_impl_files
  src/placeholder.cc
)

add_library(vw_c_api ${vw_c_public_headers} ${vw_c_impl_files})
target_include_directories(vw_c_api PUBLIC include/)
target_link_libraries(vw_c_api VowpalWabbit::vw VowpalWabbit::io)
target_compile_definitions(vw_c_api PRIVATE VWDLL_EXPORTS)

add_subdirectory(test)