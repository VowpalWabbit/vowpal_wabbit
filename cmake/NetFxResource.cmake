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