set(VW_NET_COMMON_SOURCES
  #../Properties/AssemblyInfoInternal.cs
  # fill this out for VS
)

# set(ENV{NewtonsoftDependencyValue} "Newtonsoft.Json.dll")
find_program(dotnet_t4_COMMAND t4.exe)

#(STATUS "Found dotnet-t4: ${dotnet_t4_COMMAND}"

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VW.Net.UnitTest ${CMAKE_CURRENT_SOURCE_DIR}/vw.unittest.csproj VowpalWabbit VowpalWabbit.JSON VowpalWabbit.Parallel console.net testcommon)
else()
  add_custom_target(VW.Net.UnitTest
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> -v m --nologo --no-dependencies /clp:NoSummary
      COMMENT Building VW.Net.UnitTest
      DEPENDS VowpalWabbit VowpalWabbit.JSON VowpalWabbit.Parallel console.net testcommon
      SOURCES ${VW_NET_COMMON_SOURCES})
endif()

# TODO: Enable TRX test logging
add_test(
  NAME VW.Net.UnitTest
  COMMAND
    ${DOTNET_COMMAND} test $<TARGET_FILE_DIR:vw.net.native>/VW.Net.UnitTest.dll --Platform:x64 --InIsolation "--logger:console;verbosity=default"
)
