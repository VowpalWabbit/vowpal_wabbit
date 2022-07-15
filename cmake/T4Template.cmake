find_program(netfx_t4_COMMAND TextTransform.exe REQUIRED)

# A function to set up T4 Templates and their generated code files
#
# t4_templates         : A list of T4 Template Files to annotate in the project
# generated_files_var  : Target variable to output the set of generated files

function(configure_t4 t4_templates
                      generated_files_var)

  set(generated_files)

  foreach(t4_TEMPLATE IN ITEMS ${t4_templates})
    if (${t4_TEMPLATE} MATCHES "^(.*/)?([^/]+).tt$")
      if (${CMAKE_MATCH_COUNT} EQUAL 2)
        set(t4_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_MATCH_1})
        set(t4_TARGET_FILENAME ${CMAKE_MATCH_2}.cs)
        set(t4_TEMPLATE_FILENAME ${CMAKE_MATCH_2}.tt)
      elseif (${CMAKE_MATCH_COUNT} EQUAL 1)
        set(t4_TARGET_FILENAME ${CMAKE_MATCH_1}.cs)
        set(t4_TEMPLATE_FILENAME ${CMAKE_MATCH_1}.tt)
      endif()

      set(t4_TARGET ${t4_TARGET_DIR}${t4_TARGET_FILENAME})
    endif()

    message(STATUS "Processing ${t4_TEMPLATE} => ${t4_TARGET}")

    file(MAKE_DIRECTORY ${t4_TARGET_DIR})

    exec_program(${netfx_t4_COMMAND}
      ARGS "-out ${t4_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}/${t4_TEMPLATE}")

    set(generated_files ${generated_files} ${t4_TARGET})

    set_source_files_properties(${t4_TARGET}
      PROPERTIES
      VS_CSHARP_AutoGen "True"
      VS_CSHARP_DesignTime "True"
      VS_CSHARP_DependentUpon ${t4_TEMPLATE_FILENAME})

    set_source_files_properties(${t4_TEMPLATE}
      PROPERTIES
      VS_CSHARP_Generator "TextTemplatingFileGenerator"
      VS_CSHARP_LastGenOutput ${t4_TARGET_FILENAME})
  endforeach()

  set(${generated_files_var} ${${generated_files_var}} ${generated_files} PARENT_SCOPE)
endfunction()