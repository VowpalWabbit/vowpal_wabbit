include(CSharpUtilities)
include(NetFxNuget)
include(NetFxStrongName)

# TODO: Make AssemblyInfo a configure()ed file
add_library(VowpalWabbit.JSON SHARED
  Properties/AssemblyInfo.cs
  Serializer/IVowpalWabbitJsonConverter.cs
  Serializer/JsonRawStringConverter.cs
  Serializer/JsonRawStringListConverter.cs
  Serializer/JsonTypeInspector.cs
  Serializer/PrefixedJsonReader.cs
  Serializer/VowpalWabbitJsonBuilder.cs
  Serializer/VowpalWabbitJsonException.cs
  Serializer/VowpalWabbitJsonOptimizedSerializable.cs
  Serializer/VowpalWabbitJsonReferenceResolver.cs
  Serializer/VowpalWabbitJsonSerializable.cs
  Serializer/VowpalWabbitJsonSerializer.cs
  VowpalWabbitJson.cs
  VowpalWabbitJsonThreadedPrediction.cs
  VowpalWabbitMarshalAction.cs
  ../vw_key.snk
  )

CSHARP_SET_DESIGNER_CS_PROPERTIES(
    "Properties/AssemblyInfo.cs"
)

strongname_sign_target(VowpalWabbit.JSON ${vw_DOTNET_SIGNING_KEY})

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

target_link_libraries(VowpalWabbit.JSON
  PUBLIC VowpalWabbit.Common VowpalWabbit.Core VowpalWabbit
)

target_add_nuget_references(VowpalWabbit.JSON "Newtonsoft.JSON@9.0.1")

set_target_properties(VowpalWabbit.JSON
    PROPERTIES VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
               VS_GLOBAL_ROOTNAMESPACE VW
               VS_GLOBAL_DocumentationFile "VowpalWabbit.JSON.xml"
)
