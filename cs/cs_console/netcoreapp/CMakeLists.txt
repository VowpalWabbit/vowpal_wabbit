set(VW_NET_CS_CONSOLE_SOURCES
  ../Program.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(console.net ${CMAKE_CURRENT_SOURCE_DIR}/vw.console.csproj VowpalWabbit)
else()
  add_custom_target(console.net ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building console.net
    DEPENDS VowpalWabbit.JSON
    SOURCES ${VW_NET_CS_CONSOLE_SOURCES})
  set_property(TARGET console.net APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()
