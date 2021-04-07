include(CSharpUtilities)
include(NetFxStrongName)
include(NetFxResource)

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
)

strongname_sign_target(vw_common ${vw_DOTNET_SIGNING_KEY})

# The VowpalWabbit .NET bindings generate a dynamic assembly for the serialization logic to enable
# efficient type-safe bindings. Unfortunately, in order to reference internal members, this dynamic
# assembly needs to be StrongName signed, since the assemblies in the NuGet are StrongName signed.
# To that end, we need to embed the keyfile into the NuGet, so that we can sign the generated assembly
# on the fly.
add_target_resources(vw_common ${vw_DOTNET_SIGNING_KEY})

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
