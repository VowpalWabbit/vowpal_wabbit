include(CSharpUtilities)
include(NetFxNuget)

# TODO: Make AssemblyInfo a configure()ed file
add_executable(cs_simulator
  Properties/AssemblyInfo.cs 
  Program.cs
  VowpalWabbitSimulator.cs
  App.config
)

CSHARP_SET_DESIGNER_CS_PROPERTIES(
  "Properties/AssemblyInfo.cs"
)

set (vw_DOTNET_FRAMEWORK_REFERENCES
  System
  System.Core
  System.Data
  System.Data.DataSetExtensions
  System.Net.Http
  System.Xml
  System.Xml.Linq
  Microsoft.CSharp
)

target_link_libraries(cs_simulator 
  PUBLIC vw_common vw_clr vw_cs
)

target_add_nuget_references(cs_simulator "Newtonsoft.JSON@12.0.1")

set_target_properties(cs_simulator 
    PROPERTIES VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
               VS_GLOBAL_ROOTNAMESPACE VW
               VS_GLOBAL_AutoGenerateBindingRedirects "True"
               WIN32_EXECUTABLE TRUE
)
