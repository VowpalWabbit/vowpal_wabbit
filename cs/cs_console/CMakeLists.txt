include(CSharpUtilities)
include(NetFxNuget)
include(NetFxStrongName)

# TODO: Make AssemblyInfo a configure()ed file
add_executable(cs_console
  Properties/AssemblyInfo.cs 
  Program.cs
  App.config
)

strongname_sign_target(cs_console ${vw_DOTNET_SIGNING_KEY})

CSHARP_SET_DESIGNER_CS_PROPERTIES(
  "Properties/AssemblyInfo.cs"
)

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

target_link_libraries(cs_console 
  PUBLIC VowpalWabbit.Common VowpalWabbit.Core VowpalWabbit VowpalWabbit.JSON
)

target_add_nuget_references(cs_console "Newtonsoft.JSON@9.0.1")

set_target_properties(cs_console 
    PROPERTIES VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
               VS_GLOBAL_ROOTNAMESPACE VW
               ENABLE_EXPORTS TRUE
               WIN32_EXECUTABLE TRUE
)
