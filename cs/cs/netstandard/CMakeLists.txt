set(VW_NET_CS_SOURCES
    ../ActionDependentFeature.cs
    ../Extensions.cs
    ../ObjectFactory.cs
    ../ObjectPool.cs
    ../PooledObject.cs
    ../VowpalWabbit.cs
    ../VowpalWabbitDynamic.cs
    ../VowpalWabbitExampleCollection.cs
    ../VowpalWabbitMultiLine.cs
    ../VowpalWabbitMultiLineExampleCollection.cs
    ../VowpalWabbitMultiworldTesting.cs
    ../VowpalWabbitSerializationException.cs
    ../VowpalWabbitSingleLineExampleCollection.cs
    ../VowpalWabbitSweep.cs
    ../VowpalWabbitThreadedPrediction.cs
    ../VowpalWabbitThreadedPredictionBase.cs
    ../Serializer/AnnotationInspector.cs
    ../Serializer/ConfigInspector.cs
    ../Serializer/IVowpalWabbitConverter.cs
    ../Serializer/IVowpalWabbitMultiExampleSerializerCompiler.cs
    ../Serializer/IVowpalWabbitSerializer.cs
    ../Serializer/IVowpalWabbitSerializerCompiler.cs
    ../Serializer/VowpalWabbitConstants.cs
    ../Serializer/VowpalWabbitDefaultMarshaller.cs
    ../Serializer/VowpalWabbitMarshalContext.cs
    ../Serializer/VowpalWabbitMultiExampleSerializer.cs
    ../Serializer/VowpalWabbitMultiExampleSerializerCompiler.cs
    ../Serializer/VowpalWabbitSerializerFactory.cs
    ../Serializer/VowpalWabbitSingleExampleSerializer.cs
    ../Serializer/VowpalWabbitSingleExampleSerializerCompiler.cs
    ../Serializer/Attributes/CacheableAttribute.cs
    ../Serializer/Attributes/FeatureAttribute.cs
    ../Serializer/Attributes/LabelAttribute.cs
    ../Serializer/Intermediate/EnumerizedFeature.cs
    ../Serializer/Intermediate/Feature.cs
    ../Serializer/Intermediate/Namespace.cs
    ../Serializer/Intermediate/PreHashedFeature.cs
)

find_program(dotnet_t4_COMMAND t4.exe)

#(STATUS "Found dotnet-t4: ${dotnet_t4_COMMAND}"

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit ${CMAKE_CURRENT_SOURCE_DIR}/vw.net.csproj VowpalWabbit.Common VowpalWabbit.Core)
else()
  add_custom_target(VowpalWabbit ALL
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building VowpalWabbit
      DEPENDS VowpalWabbit.Common VowpalWabbit.Core
      SOURCES ${VW_NET_CS_SOURCES})
endif()
