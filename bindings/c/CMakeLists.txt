set(vw_c_public_headers
  include/vw/experimental/placeholder.h
)

set(vw_c_impl_files
  src/placeholder.cc
)

# In order to reuse the error definition data in the C API we must make the error_data.h header available here.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../vowpalwabbit/error_data.h include/vw/experimental/error_data.h COPYONLY)
# Reuse visibility macros
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../vowpalwabbit/vwvis.h include/vw/experimental/vwvis.h COPYONLY)

add_library(vw_c_api ${vw_c_public_headers} ${vw_c_impl_files})
target_include_directories(vw_c_api PUBLIC include/ ${CMAKE_CURRENT_BINARY_DIR}/include/)
target_link_libraries(vw_c_api VowpalWabbit::vw VowpalWabbit::io)
target_compile_definitions(vw_c_api PRIVATE VWDLL_EXPORTS)

add_subdirectory(test)