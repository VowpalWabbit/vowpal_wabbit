include(FlatbufferUtils)
get_property(flatc_location TARGET flatbuffers::flatc PROPERTY LOCATION)
add_library(vw_parser_flatbuffer
  parse_example_flatbuffer.h
  parse_example_flatbuffer.cc
  parse_label.cc
)
add_library(VowpalWabbit::flatbuffer_parser ALIAS vw_parser_flatbuffer)

target_include_directories(vw_parser_flatbuffer PUBLIC
  ${FLATBUFFERS_INCLUDE_DIR}
  "$<BUILD_INTERFACE:${VW_ROOT_INCLUDE_DIR}>"
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  # Only necessary for build as when installed it will be alongside all code
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

target_compile_definitions(vw_parser_flatbuffer PUBLIC BUILD_FLATBUFFERS)

add_flatbuffer_schema(
  TARGET vw_parser_flatbuffer_generate_fb_headers_internal
  SCHEMA ${CMAKE_CURRENT_LIST_DIR}/schema/example.fbs
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/vowpalwabbit/parser/flatbuffer/generated
  FLATC_EXE ${flatc_location}
)

add_library(vw_parser_flatbuffer_generate_fb_headers INTERFACE)

# Only generate the flatbuffer headers if we are building the library
add_dependencies(vw_parser_flatbuffer_generate_fb_headers vw_parser_flatbuffer_generate_fb_headers_internal)

# vw_parser_flatbuffer_generate_fb_headers is only "linked" during build as it generates the headers. When installed they will already be there.
target_link_libraries(vw_parser_flatbuffer PUBLIC VowpalWabbit::vw $<BUILD_INTERFACE:vw_parser_flatbuffer_generate_fb_headers>)

if (VW_INSTALL)
  install(
    TARGETS vw_parser_flatbuffer
    EXPORT VowpalWabbitConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/vowpalwabbit/parser/flatbuffer/generated/example_generated.h
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit/parser/flatbuffer/generated/example_generated.h
    )
endif()
