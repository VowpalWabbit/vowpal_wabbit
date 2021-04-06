include(CSharpUtilities)
include(T4Template)

set (vw_cs_SOURCES
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

set(vw_cs_T4TEMPLATES
  Serializer/VowpalWabbitDefaultMarshallerExt.tt
)

configure_t4(${vw_cs_T4TEMPLATES} vw_cs_GENERATED_SOURCES)

# TODO: Make AssemblyInfo a configure()ed file
add_library(vw_cs SHARED
  ${vw_cs_SOURCES}
  ${vw_cs_GENERATED_SOURCES}
  ${vw_cs_T4TEMPLATES}
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

target_link_libraries(vw_cs 
  PUBLIC vw_common vw_clr
)

set_target_properties(vw_cs PROPERTIES
  VS_DOTNET_REFERENCES "${vw_DOTNET_FRAMEWORK_REFERENCES}"
  VS_GLOBAL_ROOTNAMESPACE VW
  VS_GLOBAL_AllowUnsafeBlocks "True"
)


