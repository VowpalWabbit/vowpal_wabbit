include(CSharpUtilities)
include(NetFxNuget)
include(NetFxStrongName)

# TODO: Make AssemblyInfo a configure()ed file
add_library(cs_parallel SHARED
  Properties/AssemblyInfo.cs 
  VowpalWabbitAsync.cs
  VowpalWabbitThreadedLearning.cs
)

CSHARP_SET_DESIGNER_CS_PROPERTIES(
  "Properties/AssemblyInfo.cs"
)

strongname_sign_target(cs_parallel ${vw_DOTNET_SIGNING_KEY})

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

target_add_nuget_references(cs_parallel "Microsoft.Tpl.Dataflow@4.5.24@System.Threading.Tasks.Dataflow")

target_link_libraries(cs_parallel 
  PUBLIC vw_common vw_clr vw_cs 
)

set_target_properties(cs_parallel 
    PROPERTIES VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
               VS_GLOBAL_ROOTNAMESPACE VW
)
