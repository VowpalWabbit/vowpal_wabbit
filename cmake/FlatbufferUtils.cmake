include(CMakeParseArguments)

function(add_flatbuffer_schema)
  cmake_parse_arguments(ADD_FB_SCHEMA_ARGS
    ""
    "TARGET;FLATC_EXE;OUTPUT_DIR;SCHEMA;FLATC_EXTRA_SCHEMA_ARGS"
    ""
    ${ARGN}
  )

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_TARGET)
    message(FATAL_ERROR "Missing TARGET argument to build_flatbuffers")
  endif()

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_FLATC_EXE)
    message(FATAL_ERROR "Missing FLATC_EXE argument to build_flatbuffers")
  endif()

  if(NOT EXISTS ${ADD_FB_SCHEMA_ARGS_FLATC_EXE})
    message(FATAL_ERROR "FLATC_EXE ${ADD_FB_SCHEMA_ARGS_FLATC_EXE} does not exist")
  endif()

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_OUTPUT_DIR)
    message(FATAL_ERROR "Missing OUTPUT_DIR argument to build_flatbuffers")
  endif()

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_SCHEMA)
    message(FATAL_ERROR "Missing SCHEMA argument to build_flatbuffers")
  endif()

  set(FLATC_SCHEMA_ARGS --gen-mutable)
  if(DEFINED ADD_FB_SCHEMA_ARGS_FLATC_EXTRA_SCHEMA_ARGS)
    set(FLATC_SCHEMA_ARGS ${ADD_FB_SCHEMA_ARGS_FLATC_EXTRA_SCHEMA_ARGS} ${FLATC_SCHEMA_ARGS})
  endif()

  get_filename_component(filename ${ADD_FB_SCHEMA_ARGS_SCHEMA} NAME_WE)
  set(generated_file_name ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR}/${filename}_generated.h)
  add_custom_command(
      OUTPUT ${generated_file_name}
      COMMAND ${ADD_FB_SCHEMA_ARGS_FLATC_EXE} ${FLATC_SCHEMA_ARGS} -o ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR} -c ${ADD_FB_SCHEMA_ARGS_SCHEMA}
      DEPENDS ${ADD_FB_SCHEMA_ARGS_SCHEMA}
  )

  add_custom_target(${ADD_FB_SCHEMA_ARGS_TARGET} DEPENDS ${generated_file_name})
endfunction()
