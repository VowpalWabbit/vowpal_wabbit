set(VW_NET_COMMON_SOURCES
  #../Properties/AssemblyInfoInternal.cs
  # fill this out for VS
)

find_program(dotnet_t4_COMMAND t4.exe)

#(STATUS "Found dotnet-t4: ${dotnet_t4_COMMAND}"

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.JSON ${CMAKE_CURRENT_SOURCE_DIR}/vw.json.csproj VowpalWabbit)
else()
  add_custom_target(VowpalWabbit.JSON
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building VowpalWabbit.JSON
      DEPENDS VowpalWabbit
      SOURCES ${VW_NET_COMMON_SOURCES})
endif()