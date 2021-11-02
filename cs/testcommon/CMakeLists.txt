include(CSharpUtilities)
include(NetFxStrongName)

# TODO: Make AssemblyInfo a configure()ed file
# TODO: Why does this project exist?
add_library(cs_testcommon SHARED
  Properties/AssemblyInfo.cs 
  ITestRunner.cs
)

strongname_sign_target(cs_testcommon ${vw_DOTNET_SIGNING_KEY})

CSHARP_SET_DESIGNER_CS_PROPERTIES(
    "Properties/AssemblyInfo.cs"
)

set (vw_DOTNET_FRAMEWORK_REFERENCES
    System
    System.Core
    System.Data
    System.Data.DataSetExtensions
    System.Xml
    System.Xml.Linq
    Microsoft.CSharp
)

set_target_properties(cs_testcommon PROPERTIES
    VS_DOTNET_REFERENCES ${vw_DOTNET_FRAMEWORK_REFERENCES}
    VS_GLOBAL_ROOTNAMESPACE VW
)
