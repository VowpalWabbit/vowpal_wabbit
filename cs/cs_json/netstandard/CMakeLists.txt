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

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.JSON ${CMAKE_CURRENT_SOURCE_DIR}/vw.json.csproj VowpalWabbit)
else()
  add_custom_target(VowpalWabbit.JSON ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT "Building VowpalWabbit.JSON"
    DEPENDS VowpalWabbit
    SOURCES ${VW_NET_CS_JSON_SOURCES})
  add_custom_target(VowpalWabbit.JSON.Ref ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native>/ref --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary -property:ProduceOnlyReferenceAssembly=true
    COMMENT "Building VowpalWabbit.JSON Reference Assembly"
    DEPENDS VowpalWabbit.Ref VowpalWabbit.JSON
    SOURCES ${VW_NET_CS_JSON_SOURCES})
  set_property(TARGET VowpalWabbit.JSON APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()

install(
  FILES
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.JSON.dll"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.JSON.pdb"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.JSON.deps.json"
  DESTINATION "./runtimes/${NUGET_RUNTIME_ID}/lib/netstandard2.1"
)
install(
  FILES "$<TARGET_FILE_DIR:vw.net.native>/ref/VowpalWabbit.JSON.dll"
  DESTINATION "./ref/netstandard2.1"
)
