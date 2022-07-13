# Generate header file
add_flatbuffer_schema(
  TARGET fbmodel
  SCHEMAS   "${CMAKE_CURRENT_LIST_DIR}/schema/gd.fbs" 
            "${CMAKE_CURRENT_LIST_DIR}/schema/model.fbs"
  OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/vw/fb_model/generated/"
  FLATC_EXE ${flatc_location}
)
add_library(fb_generate_model_headers INTERFACE)
add_dependencies(fb_generate_model_headers fbmodel)
target_include_directories(fb_generate_model_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/include/)

set(vw_fb_model_sources
  include/vw/fb_model/parse_model_flatbuffer.h
  src/parse_model_flatbuffer.cc
)

vw_add_library(
    NAME "fb_model"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_fb_model_sources}
    PUBLIC_DEPS vw_core $<BUILD_INTERFACE:fb_generate_model_headers>
    DESCRIPTION "Model Parser that implementation flatbuffer model file."
    EXCEPTION_DESCRIPTION "No"
)
target_include_directories(vw_fb_model PUBLIC ${FLATBUFFERS_INCLUDE_DIR})

# Install the generated header
# if(VW_INSTALL)
#   install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# endif()

vw_add_test_executable(
  FOR_LIB "fb_model"
  SOURCES "tests/flatbuffer_model_test.cc"
  EXTRA_DEPS vw_core
)