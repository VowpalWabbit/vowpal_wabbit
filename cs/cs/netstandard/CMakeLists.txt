set(VW_NET_COMMON_SOURCES
  #../Properties/AssemblyInfoInternal.cs
  # fill this out for VS
)

find_program(dotnet_t4_COMMAND t4.exe)

#(STATUS "Found dotnet-t4: ${dotnet_t4_COMMAND}"

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit ${CMAKE_CURRENT_SOURCE_DIR}/vw.net.csproj VowpalWabbit.Common VowpalWabbit.Core)
else()
  add_custom_target(VowpalWabbit
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building VowpalWabbit
      DEPENDS VowpalWabbit.Common VowpalWabbit.Core
      SOURCES ${VW_NET_COMMON_SOURCES})
endif()