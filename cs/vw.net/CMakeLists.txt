set(VW_NET_CORE_SOURCES
  
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VowpalWabbit.Core ${CMAKE_CURRENT_SOURCE_DIR}/vw.net.core.csproj vw.net.native VowpalWabbit.Common)
  message(STATUS "!!!! using msproject")
else()
  add_custom_target(VowpalWabbit.Core
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building VowpalWabbit.Core
      DEPENDS vw.net.native VowpalWabbit.Common
      SOURCES ${VW_NET_CORE_SOURCES})
endif()
