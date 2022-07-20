set(VW_NET_CS_JSON_SOURCES
    ../VowpalWabbitJson.cs
    ../VowpalWabbitJsonThreadedPrediction.cs
    ../VowpalWabbitMarshalAction.cs
    ../Serializer/IVowpalWabbitJsonConverter.cs
    ../Serializer/JsonRawStringConverter.cs
    ../Serializer/JsonRawStringListConverter.cs
    ../Serializer/JsonTypeInspector.cs
    ../Serializer/PrefixedJsonReader.cs
    ../Serializer/VowpalWabbitJsonBuilder.cs
    ../Serializer/VowpalWabbitJsonException.cs
    ../Serializer/VowpalWabbitJsonOptimizedSerializable.cs
    ../Serializer/VowpalWabbitJsonReferenceResolver.cs
    ../Serializer/VowpalWabbitJsonSerializable.cs
    ../Serializer/VowpalWabbitJsonSerializer.cs
)

find_program(dotnet_t4_COMMAND t4.exe)

#(STATUS "Found dotnet-t4: ${dotnet_t4_COMMAND}"

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.JSON ${CMAKE_CURRENT_SOURCE_DIR}/vw.json.csproj VowpalWabbit)
else()
  add_custom_target(VowpalWabbit.JSON ALL
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building VowpalWabbit.JSON
      DEPENDS VowpalWabbit
      SOURCES ${VW_NET_CS_JSON_SOURCES})
endif()
