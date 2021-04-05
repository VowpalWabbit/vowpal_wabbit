include(CSharpUtilities)

# TODO: Make AssemblyInfo a configure()ed file
add_library(vw_common SHARED
  Properties/AssemblyInfo.cs 
  Reflection/InspectionHelper.cs
  Reflection/ReflectionHelper.cs
  Reflection/TypeDistance.cs
  Reflection/TypeMatch.cs
  Serializer/FeatureExpression.cs
  Serializer/LabelExpression.cs
  Serializer/PropertyConfiguration.cs
  Serializer/Schema.cs
  Serializer/StringProcessing.cs
  Bag.cs
  VowpalWabbitDecisionServiceInteractionHeader.cs
  ../vw_key.snk
  )

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

set_target_properties(vw_common PROPERTIES
    VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
    VS_GLOBAL_ROOTNAMESPACE VW
)
