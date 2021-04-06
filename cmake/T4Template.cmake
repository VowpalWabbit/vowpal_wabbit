# A function to set up T4 Templates and their generated code files
#
# t4_templates         : A list of T4 Template Files to annotate in the project
# generated_files_var  : Target variable to output the set of generated files

function(configure_t4 t4_templates 
                      generated_files_var)
  
  set(generated_files)

  foreach(t4_TEMPLATE IN ITEMS ${t4_templates})
    string(REGEX REPLACE "^(.+)/(.+).tt$" "\\2.tt" t4_TEMPLATE_FILENAME ${t4_TEMPLATE})
    string(REGEX REPLACE "^(.+)\\.tt$" "\\1.cs" t4_TARGET ${t4_TEMPLATE})
    string(REGEX REPLACE "^(.+)/(.+).tt$" "\\2.cs" t4_TARGET_FILENAME ${t4_TEMPLATE})
    message(VERBOSE "Processing ${t4_TEMPLATE} => ${t4_TARGET}")

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