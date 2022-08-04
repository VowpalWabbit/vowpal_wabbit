set(VW_NET_CS_PARALLEL_SOURCES
    ../VowpalWabbitAsync.cs
    ../VowpalWabbitThreadedLearning.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.Parallel ${CMAKE_CURRENT_SOURCE_DIR}/vw.parallel.csproj VowpalWabbit)
else()
  add_custom_target(VowpalWabbit.Parallel ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building VowpalWabbit.Parallel
    DEPENDS VowpalWabbit
    SOURCES ${VW_NET_CS_PARALLEL_SOURCES})
  set_property(TARGET VowpalWabbit.Parallel APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()

install(
  FILES
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Parallel.dll"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Parallel.pdb"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Parallel.deps.json"
  DESTINATION "./runtimes/${NUGET_RUNTIME_ID}/lib/netstandard2.1"
)
install(
  FILES
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Parallel.dll"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Parallel.pdb"
    "$<TARGET_FILE_DIR:vw.net.native>/VowpalWabbit.Parallel.deps.json"
  DESTINATION "./ref/netstandard2.1"
)
