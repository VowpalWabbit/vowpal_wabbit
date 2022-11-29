set(VW_NET_TESTCOMMON_SOURCES
  ../ITestRunner.cs
  ../RunTestsGenerator.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(testcommon ${CMAKE_CURRENT_SOURCE_DIR}/vw.testcommon.csproj)
else()
  add_custom_target(testcommon ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building testcommon
    SOURCES ${VW_NET_TESTCOMMON_SOURCES})
  set_property(TARGET testcommon APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()
