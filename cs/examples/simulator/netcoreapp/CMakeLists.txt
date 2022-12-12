set(VW_NET_EXAMPLE_SIMULATOR_SOURCES
    ../Program.cs
    ../VowpalWabbitSimulator.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(simulator ${CMAKE_CURRENT_SOURCE_DIR}/vw.simulator.csproj VowpalWabbit)
else()
  add_custom_target(simulator ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building Simulator
    DEPENDS VowpalWabbit.JSON
    SOURCES ${VW_NET_EXAMPLE_SIMULATOR_SOURCES})
  set_property(TARGET simulator APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()
