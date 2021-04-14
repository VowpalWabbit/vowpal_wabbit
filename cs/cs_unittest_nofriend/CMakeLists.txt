include(CSharpUtilities)
include(FindVSTest)
include(NetFxNuget)
include(NetFxStrongName)

# TODO: Make AssemblyInfo a configure()ed file
add_library(cs_unittest_nofriend SHARED
  Properties/AssemblyInfo.cs 
  TestSerializer.cs
)

strongname_sign_target(cs_unittest_nofriend ${vw_DOTNET_SIGNING_KEY})

CSHARP_SET_DESIGNER_CS_PROPERTIES(
  "Properties/AssemblyInfo.cs"
)

set (vw_DOTNET_FRAMEWORK_REFERENCES
  System
  System.Data
  System.Runtime.Caching
  System.Runtime.Serialization
  System.Windows
  System.Xml
  Microsoft.VisualStudio.QualityTools.UnitTestFramework
)

target_link_libraries(cs_unittest_nofriend 
  PUBLIC VowpalWabbit.Common VowpalWabbit.Core VowpalWabbit
)

set_target_properties(cs_unittest_nofriend 
  PROPERTIES 
  VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
  VS_GLOBAL_ROOTNAMESPACE cs_unittest_nofriend
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_GLOBAL_TestProjectType "UnitTest"
)

get_property(cs_unittest_nofriend_OUTPUT_DIRECTORY TARGET cs_unittest_nofriend PROPERTY RUNTIME_OUTPUT_DIRECTORY)

add_test(NAME cs_unittest_nofriend
        COMMAND
          ${VSTEST_COMMAND} $<TARGET_FILE:cs_unittest_nofriend>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})