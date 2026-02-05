set(VW_NET_CORE_SOURCES
  ApiStatus.cs
  IVowpalWabbitExamplePool.cs
  IVowpalWabbitLabelComparator.cs
  IVowpalWabbitPredictionFactory.cs
  Labels/ILabel.cs
  MultiExAdapter.cs
  SpanningTreeClr.cs
  VowpalWabbit.cs
  VowpalWabbitArguments.cs
  VowpalWabbitBase.cs
  VowpalWabbitContextualBanditUtil.cs
  VowpalWabbitExample.cs
  VowpalWabbitExampleBuilder.cs
  VowpalWabbitException.cs
  VowpalWabbitFeature.cs
  VowpalWabbitModel.cs
  VowpalWabbitNamespace.cs
  VowpalWabbitNamespaceBuilder.cs
  VowpalWabbitPerformanceStatistics.cs
  VowpalWabbitSettings.cs
  VwException.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.Core ${CMAKE_CURRENT_SOURCE_DIR}/vw.net.core.csproj vw.net.native VowpalWabbit.Common)
  message(STATUS "!!!! using msproject")
else()
  add_custom_target(VowpalWabbit.Core
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT "Building VowpalWabbit.Core"
    DEPENDS vw.net.native VowpalWabbit.Common
    SOURCES ${VW_NET_CORE_SOURCES})
  add_custom_target(VowpalWabbit.Core.Ref
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native>/ref --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary -property:ProduceOnlyReferenceAssembly=true
    COMMENT "Building VowpalWabbit.Core Reference Assembly"
    DEPENDS vw.net.native VowpalWabbit.Common.Ref VowpalWabbit.Core
    SOURCES ${VW_NET_CORE_SOURCES})
  set_property(TARGET VowpalWabbit.Core APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()

install(
  FILES
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Core.dll"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Core.pdb"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Core.deps.json"
  DESTINATION "./runtimes/${NUGET_RUNTIME_ID}/lib/netstandard2.1"
)
install(
  FILES "$<TARGET_FILE_DIR:vw.net.native>/ref/VowpalWabbit.Core.dll"
  DESTINATION "./ref/netstandard2.1"
)
