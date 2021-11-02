# A function to add a set of files as EmbeddedResources into a .NET Framework CMake TARGET
#
# target_name     : The CMake TARGET to update
# resource_files  : A list of items to embed into the generated assembly
# 

function(add_target_resources target_name 
                              resource_files)
  set_property(TARGET ${target_name} APPEND PROPERTY SOURCES ${resource_files})

  set_source_files_properties(${resource_files}
    PROPERTIES
    VS_TOOL_OVERRIDE "EmbeddedResource"
  )

endfunction()

# A function to a set of files as ContentResources into a .NET Framework CMake TARGET
# 
# target_name            : The CMake TARGET to update
# content_files          : A list of content items to add to the target

function(add_target_content target_name
                            content_files)

  set_property(TARGET ${target_name} APPEND PROPERTY SOURCES ${resource_files})

  set_source_files_properties(${resource_files}
    PROPERTIES
    VS_TOOL_OVERRIDE "Content"
    VS_CSHARP_CopyToOutputDirectory "PreserveNewest"
  )

endfunction()