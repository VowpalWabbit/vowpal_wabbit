set(VW_NET_COMMON_SOURCES
  ../Reflection/InspectionHelper.cs
  ../Reflection/ReflectionHelper.cs
  ../Reflection/TypeDistance.cs
  ../Reflection/TypeMatch.cs
  ../Serializer/FeatureExpression.cs
  ../Serializer/LabelExpression.cs
  ../Serializer/PropertyConfiguration.cs
  ../Serializer/Schema.cs
  ../Serializer/StringProcessing.cs
  ../Bag.cs
  ../VowpalWabbitDecisionServiceInteractionHeader.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.Common ${CMAKE_CURRENT_SOURCE_DIR}/vw.common.csproj)
else()
  add_custom_target(VowpalWabbit.Common ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT "Building VowpalWabbit.Common"
    SOURCES ${VW_NET_COMMON_SOURCES})
  add_custom_target(VowpalWabbit.Common.Ref ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native>/ref --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary -property:ProduceOnlyReferenceAssembly=true
    COMMENT "Building VowpalWabbit.Common Reference Assembly"
    DEPENDS VowpalWabbit.Common
    SOURCES ${VW_NET_COMMON_SOURCES})
  set_property(TARGET VowpalWabbit.Common APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()

install(
  FILES
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Common.dll"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Common.pdb"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Common.deps.json"
  DESTINATION "./runtimes/${NUGET_RUNTIME_ID}/lib/netstandard2.1"
)
install(
  FILES "$<TARGET_FILE_DIR:vw.net.native>/ref/VowpalWabbit.Common.dll"
  DESTINATION "./ref/netstandard2.1"
)
