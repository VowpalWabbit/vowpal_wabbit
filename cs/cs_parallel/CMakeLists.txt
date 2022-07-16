include(CSharpUtilities)
include(NetFxNuget)
include(NetFxStrongName)

# TODO: Make AssemblyInfo a configure()ed file
add_library(VowpalWabbit.Parallel SHARED
  Properties/AssemblyInfo.cs
  VowpalWabbitAsync.cs
  VowpalWabbitThreadedLearning.cs
)

CSHARP_SET_DESIGNER_CS_PROPERTIES(
  "Properties/AssemblyInfo.cs"
)

strongname_sign_target(VowpalWabbit.Parallel ${vw_DOTNET_SIGNING_KEY})

set (vw_DOTNET_FRAMEWORK_REFERENCES
  System
  System.Core
  System.Data
  System.Data.DataSetExtensions
  System.Runtime.Caching
  System.Xml
  System.Xml.Linq
  Microsoft.CSharp
)

target_add_nuget_references(VowpalWabbit.Parallel "Microsoft.Tpl.Dataflow@4.5.24@System.Threading.Tasks.Dataflow")

target_link_libraries(VowpalWabbit.Parallel
  PUBLIC VowpalWabbit.Common VowpalWabbit.Core VowpalWabbit
)

target_compile_options(VowpalWabbit.Parallel PRIVATE
  $<$<CONFIG:Release>:/debug:pdbonly>
  $<$<CONFIG:Debug>:/debug:full>
)

set_target_properties(VowpalWabbit.Parallel
    PROPERTIES VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
               VS_GLOBAL_ROOTNAMESPACE VW
               VS_GLOBAL_DocumentationFile "VowpalWabbit.Parallel.xml"
)
