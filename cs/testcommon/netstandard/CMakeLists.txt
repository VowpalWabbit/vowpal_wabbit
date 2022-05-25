set(VW_NET_TESTCOMMON_SOURCES
  ../ITestRunner.cs
  ../RunTestsGenerator.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(testcommon ${CMAKE_CURRENT_SOURCE_DIR}/vw.testcommon.csproj)
else()
  add_custom_target(testcommon
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building testcommon
      SOURCES ${VW_NET_TESTCOMMON_SOURCES})
endif()