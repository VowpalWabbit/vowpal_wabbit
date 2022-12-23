# Generate header file
add_flatbuffer_schema(
  TARGET fbschemas
  SCHEMAS "${CMAKE_CURRENT_LIST_DIR}/schema/example.fbs"
  OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/vw/fb_parser/generated/"
  FLATC_EXE ${flatc_location}
)
add_library(fb_generate_headers INTERFACE)
add_dependencies(fb_generate_headers fbschemas)
target_include_directories(fb_generate_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/include/)

set(vw_fb_parser_sources
    include/vw/fb_parser/parse_example_flatbuffer.h
    src/parse_example_flatbuffer.cc
    src/parse_label.cc
)

vw_add_library(
    NAME "fb_parser"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_fb_parser_sources}
    PUBLIC_DEPS vw_core $<BUILD_INTERFACE:fb_generate_headers>
    DESCRIPTION "Parser implementation that reads flatbuffer examples. Disabled by default. Enable with `BUILD_FLATBUFFERS`"
    EXCEPTION_DESCRIPTION "Yes"
)
target_include_directories(vw_fb_parser PUBLIC ${FLATBUFFERS_INCLUDE_DIR})

# Install the generated header
# if(VW_INSTALL)
#   install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# endif()

vw_add_test_executable(
  FOR_LIB "fb_parser"
  SOURCES "tests/flatbuffer_parser_test.cc"
  EXTRA_DEPS vw_core vw_test_common
)