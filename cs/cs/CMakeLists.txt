include(CSharpUtilities)
include(T4Template)
include(NetFxStrongName)

set (VowpalWabbit_SOURCES
  Properties/AssemblyInfo.cs
  Properties/AssemblyInfoInternal.cs # Things seem to work without this, but going to keep it here to be 1:1 with previous
  Serializer/Attributes/CacheableAttribute.cs
  Serializer/Attributes/FeatureAttribute.cs
  Serializer/Attributes/LabelAttribute.cs
  Serializer/Intermediate/EnumerizedFeature.cs
  Serializer/Intermediate/Feature.cs
  Serializer/Intermediate/Namespace.cs
  Serializer/Intermediate/PreHashedFeature.cs
  Serializer/AnnotationInspector.cs
  Serializer/ConfigInspector.cs
  Serializer/IVowpalWabbitConverter.cs
  Serializer/IVowpalWabbitMultiExampleSerializerCompiler.cs
  Serializer/IVowpalWabbitSerializer.cs
  Serializer/IVowpalWabbitSerializerCompiler.cs
  Serializer/VowpalWabbitConstants.cs
  Serializer/VowpalWabbitDefaultMarshaller.cs
  Serializer/VowpalWabbitMarshalContext.cs
  Serializer/VowpalWabbitMultiExampleSerializer.cs
  Serializer/VowpalWabbitMultiExampleSerializerCompiler.cs
  Serializer/VowpalWabbitSerializerFactory.cs
  Serializer/VowpalWabbitSingleExampleSerializer.cs
  Serializer/VowpalWabbitSingleExampleSerializerCompiler.cs
  ActionDependentFeature.cs
  Extensions.cs
  ObjectFactory.cs
  ObjectPool.cs
  PooledObject.cs
  VowpalWabbit.cs
  VowpalWabbitDynamic.cs
  VowpalWabbitExampleCollection.cs
  VowpalWabbitMultiLine.cs
  VowpalWabbitMultiLineExampleCollection.cs
  VowpalWabbitMultiworldTesting.cs
  VowpalWabbitSerializationException.cs
  VowpalWabbitSingleLineExampleCollection.cs
  VowpalWabbitSweep.cs
  VowpalWabbitThreadedPrediction.cs
  VowpalWabbitThreadedPredictionBase.cs
)

set(VowpalWabbit_T4TEMPLATES
  Serializer/VowpalWabbitDefaultMarshallerExt.tt
)

configure_t4(${VowpalWabbit_T4TEMPLATES} VowpalWabbit_GENERATED_SOURCES)

# TODO: Make AssemblyInfo a configure()ed file
add_library(VowpalWabbit SHARED
  ${VowpalWabbit_SOURCES}
  ${VowpalWabbit_GENERATED_SOURCES}
  ${VowpalWabbit_T4TEMPLATES}
)

strongname_sign_target(VowpalWabbit ${vw_DOTNET_SIGNING_KEY})

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

target_compile_options(VowpalWabbit PRIVATE
  $<$<CONFIG:Release>:/debug:pdbonly>
  $<$<CONFIG:Debug>:/debug:full>
)

target_link_libraries(VowpalWabbit
  PUBLIC VowpalWabbit.Common VowpalWabbit.Core
)

set_target_properties(VowpalWabbit PROPERTIES
  VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
  VS_GLOBAL_ROOTNAMESPACE VW
  VS_GLOBAL_AllowUnsafeBlocks "True"
  VS_GLOBAL_DocumentationFile "VowpalWabbit.xml"
)
