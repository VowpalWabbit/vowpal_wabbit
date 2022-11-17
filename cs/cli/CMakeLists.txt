include(CSharpUtilities)
include(NetFxNuget)
include(NetFxStrongName)

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
  resource.h
)

set(vw_CLI_SOURCES
  AssemblyInfo.cpp
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
add_library(VowpalWabbit.Core SHARED
  ${vw_CLI_HEADERS}
  ${vw_CLI_SOURCES}
  Resource.rc
)

# This ensures that the assembly is built with /clr. It is empty because we have mixed native and
# managed C++. It is important to set this property ahead of doing StrongName signing, as it is
# used to determine whether the assembly is build from a vcxproj vs. a csproj.
set_property(TARGET VowpalWabbit.Core PROPERTY COMMON_LANGUAGE_RUNTIME "")

strongname_sign_target(VowpalWabbit.Core ${vw_DOTNET_SIGNING_KEY})

set (vw_DOTNET_FRAMEWORK_REFERENCES
    System
    System.Core
)

set (VowpalWabbit.Core_NUGET_REFERENCES
  Newtonsoft.Json@9.0.1
)

target_add_nuget_references(VowpalWabbit.Core "${VowpalWabbit.Core_NUGET_REFERENCES}")

# Define dependencies.
target_link_libraries(VowpalWabbit.Core
    PUBLIC VowpalWabbit.Common
    PRIVATE vw_core vw_spanning_tree
)

target_compile_options(VowpalWabbit.Core PRIVATE
  /doc
  $<$<CONFIG:Release>:/FdVowpalWabbit.Core.pdb>
  $<$<CONFIG:Debug>:/FdVowpalWabbit.Core.pdb>
)

target_link_options(VowpalWabbit.Core PRIVATE
  $<$<CONFIG:Release>:/DEBUG>
  $<$<CONFIG:Debug>:/DEBUG>
)

set_target_properties(VowpalWabbit.Core PROPERTIES
    VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
    VS_DOTNET_TARGET_FRAMEWORK_VERSION ${CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION}
    VS_GLOBAL_ROOTNAMESPACE VowpalWabbit.Core
    VS_GLOBAL_CLRSupport "True"
)

install(TARGETS VowpalWabbit.Core
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}/${VW_NET_FRAMEWORK_TFM}"
)
