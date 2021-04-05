include(CSharpUtilities)

set(vw_CLI_HEADERS
  clr_io.h
  spanning_tree_clr.h
  vowpalwabbit.h
  vw_arguments.h
  vw_base.h
  vw_builder.h
  vw_cbutil.h
  vw_clr.h
  vw_example.h
  vw_interface.h
  vw_label.h
  vw_labelcomparator.h
  vw_model.h
  vw_prediction.h
  vw_settings.h
)

set(vw_CLI_SOURCES
  clr_io.cpp
  spanning_tree_clr.cpp
  vowpalwabbit.cpp
  vw_base.cpp
  vw_builder.cpp
  vw_cbutil.cpp
  vw_example.cpp
  vw_exception.cpp
  vw_model.cpp
  vw_prediction.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${vw_CLI_HEADERS} ${vw_CLI_SOURCES})

# TODO: Make AssemblyInfo a configure()ed file
add_library(vw_clr SHARED
  ${vw_CLI_HEADERS}
  ${vw_CLI_SOURCES}
  )

# Define dependencies.
target_link_libraries(vw_clr 
    PUBLIC vw_common 
    PRIVATE vw
    # Workaround an issue where RapidJSON needed to be exported tom install the target. This is
    # actually a private dependency and so do not "link" when processing targets for installation.
    # https://gitlab.kitware.com/cmake/cmake/issues/15415
    $<BUILD_INTERFACE:RapidJSON>
)

set_property(TARGET vw_clr PROPERTY COMMON_LANGUAGE_RUNTIME "")

set (vw_DOTNET_FRAMEWORK_REFERENCES
    System
    )

find_program(NuGet NAMES nuget) # TODO: Ensure this works in VW / under init.cmd
exec_program(${NuGet}
    ARGS install "Newtonsoft.Json" -Version 9.0.1 -OutputDirectory ${CMAKE_BINARY_DIR}/packages)

set_property(TARGET vw_clr PROPERTY 
    VS_DOTNET_REFERENCE_Newtonsoft.Json 
    ${CMAKE_BINARY_DIR}/packages/EntityFramework.9.0.1/lib/net45/EntityFramework.dll)
    
set_target_properties(vw_clr PROPERTIES
    VS_DOTNET_REFERENCES ${vw_DOTNET_FRAMEWORK_REFERENCES}
    VS_GLOBAL_ROOTNAMESPACE vw_clr
)