set(VW_NET_CS_PARALLEL_SOURCES
    ../VowpalWabbitAsync.cs
    ../VowpalWabbitThreadedLearning.cs
)

find_program(dotnet_t4_COMMAND t4.exe)

#(STATUS "Found dotnet-t4: ${dotnet_t4_COMMAND}"

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.Parallel ${CMAKE_CURRENT_SOURCE_DIR}/vw.parallel.csproj VowpalWabbit)
else()
  add_custom_target(VowpalWabbit.Parallel ALL
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building VowpalWabbit.Parallel
      DEPENDS VowpalWabbit
      SOURCES ${VW_NET_CS_PARALLEL_SOURCES})
endif()
