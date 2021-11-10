
include(CSharpUtilities)
include(FindVSTest)
include(NetFxNuget)
include(NetFxResource)
include(NetFxStrongName)
include(T4Template)


set(vw_CS_UNITTEST_SOURCES
  Properties/AssemblyInfo.cs
  BaseData.cs
  CbAdfData.cs
  RunTestsHelper.cs
  Test1and2.cs
  Test3.cs
  TestAllReduce.cs
  TestAntlr.cs
  TestArguments.cs
  TestBase.cs
  TestCbAdf.cs
  TestConfidence.cs
  TestConfigInspector.cs
  TestConsole.cs
  TestDynamic.cs
  TestErrorListener.cs
  TestExampleCache.cs
  TestExpansion.cs
  TestFeatureExtraction.cs
  TestJson.cs
  TestJsonDict.cs
  TestJsonDirect.cs
  TestLabels.cs
  TestLeak.cs
  TestManagedHash.cs
  TestMarshalling.cs
  TestMarshallingOverflow.cs
  TestModelLoading.cs
  TestMultiClassPredictionFactory.cs
  TestMultiworldTesting.cs
  TestNull.cs
  TestPooling.cs
  TestRunner.cs
  TestSearch.cs
  TestSerializer.cs
  TestTelemetryProcessor.cs
  TestTracing.cs
  TestWiki.cs
  TestWrapper.cs
  TrainSet0002Dat.cs
  TrainSetCs_testLdf.cs
  VowpalWabbitExampleJsonValidator.cs
  VowpalWabbitExampleValidator.cs
  VowpalWabbitListenerToEvents.cs
  VowpalWabbitStdErrPerformanceStatistics.cs
  VWTestHelper.cs
  VowpalWabbit.g4
)

set(vw_CS_UNITTEST_T4TEMPLATES
  RunTests.tt
  TestMarshalNumeric.tt
)

set(vw_CS_UNITTEST_COPIED_CONTENT
  json/test_array.json
  json/test_newline.json
)

configure_t4("${vw_CS_UNITTEST_T4TEMPLATES}" vw_CS_UNITTEST_GENERATED_SOURCES)

# TODO: Make AssemblyInfo a configure()ed file
add_library(cs_unittest SHARED
  ${vw_CS_UNITTEST_SOURCES}
  ${vw_CS_UNITTEST_T4TEMPLATES}
  ${vw_CS_UNITTEST_GENERATED_SOURCES}
)

add_target_content(cs_unittest "${vw_CS_UNITTEST_COPIED_CONTENT}")

strongname_sign_target(cs_unittest ${vw_DOTNET_SIGNING_KEY})

# TODO: Change this to happen during configuration?
set_source_files_properties(VowpalWabbit.g4
  PROPERTIES
  VS_CSHARP_Generator "MSBuild:Compile"
  VS_CSHARP_Listener "True"
  VS_CSHARP_Visitor "False"
  VS_TOOL_OVERRIDE "Antlr4")


CSHARP_SET_DESIGNER_CS_PROPERTIES(
    "Properties/AssemblyInfo.cs"
)

set (vw_DOTNET_FRAMEWORK_REFERENCES
  System
  System.Data
  System.Runtime.Caching
  System.Runtime.Serialization
  System.ServiceModel
  System.Windows
  System.Xml
  Microsoft.VisualStudio.QualityTools.UnitTestFramework
  WindowsBase
)

set(cs_unittest_NUGET_REFERENCES
  Antlr4.Runtime@4.6.4
  Microsoft.ApplicationInsights@2.2.0
  Microsoft.ApplicationInsights.PerfCounterCollector@2.2.0@Microsoft.AI.PerfCounterCollector
  Microsoft.Azure.KeyVault.Core@2.0.4
  Microsoft.Azure.ServiceBus.EventProcessorHost@2.2.8@Microsoft.ServiceBus.Messaging.EventProcessorHost
  Microsoft.Data.Edm@5.8.1
  Microsoft.Data.OData@5.8.1
  Microsoft.Data.Services.Client@5.8.1
  morelinq@1.4.0
  Newtonsoft.Json@9.0.1
  System.Reactive.Core@3.1.1
  System.Reactive.Interfaces@3.1.1
  System.Reactive.Linq@3.1.1
  System.Reactive.PlatformServices@3.1.1
  System.Reactive.Windows.Threading@3.1.1
  System.Spatial@5.8.1
  Microsoft.Tpl.Dataflow@4.5.24@System.Threading.Tasks.Dataflow
  WindowsAzure.ServiceBus@3.4.3@Microsoft.ServiceBus
  WindowsAzure.Storage@7.2.1@Microsoft.WindowsAzure.Storage
)

set(cs_unittest_NUGET_IMPORTS
  Antlr4.CodeGenerator@4.6.4@build/Antlr4.CodeGenerator.props
  Antlr4.CodeGenerator@4.6.4@build/Antlr4.CodeGenerator.targets
)

target_add_nuget_references(cs_unittest
  "${cs_unittest_NUGET_REFERENCES}")

target_add_nuget_imports(cs_unittest
  "${cs_unittest_NUGET_IMPORTS}")

target_link_libraries(cs_unittest
  PUBLIC VowpalWabbit.Common VowpalWabbit.Core vw_cs VowpalWabbit.JSON VowpalWabbit.Parallel cs_console cs_testcommon
)

set_target_properties(cs_unittest
  PROPERTIES
  VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6" # This needs to be 4.6 due to ApplicationInsights
  VS_GLOBAL_ROOTNAMESPACE cs_unittest
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_GLOBAL_TestProjectType "UnitTest"
)

get_property(cs_unittest_RUNTIME_OUTPUT_DIRECTORY TARGET cs_unittest PROPERTY RUNTIME_OUTPUT_DIRECTORY)

add_test(
  NAME cs_unittest
    COMMAND
      ${VSTEST_COMMAND} $<TARGET_FILE:cs_unittest>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)