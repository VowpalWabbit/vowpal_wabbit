set(VW_NET_COMMON_SOURCES
  #../Properties/AssemblyInfoInternal.cs
  # fill this out for VS
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(console.net ${CMAKE_CURRENT_SOURCE_DIR}/vw.console.csproj VowpalWabbit)
else()
  add_custom_target(console.net
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building console.net
      DEPENDS VowpalWabbit.JSON
      SOURCES ${VW_NET_COMMON_SOURCES})
endif()