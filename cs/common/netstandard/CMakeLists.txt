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
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building VowpalWabbit.Common
    SOURCES ${VW_NET_COMMON_SOURCES})
  set_property(TARGET VowpalWabbit.Common APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()
