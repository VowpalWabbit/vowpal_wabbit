set(VW_NET_UNITTEST_SOURCES
    ../BaseData.cs
    ../CbAdfData.cs
    ../RunTestsHelper.cs
    ../Test1and2.cs
    ../Test3.cs
    ../TestAllReduce.cs
    ../TestAntlr.cs
    ../TestArguments.cs
    ../TestBase.cs
    ../TestCbAdf.cs
    ../TestConfidence.cs
    ../TestConfigInspector.cs
    ../TestConsole.cs
    ../TestDynamic.cs
    ../TestErrorListener.cs
    ../TestExampleCache.cs
    ../TestExpansion.cs
    ../TestFeatureExtraction.cs
    ../TestJson.cs
    ../TestJsonDict.cs
    ../TestJsonDirect.cs
    ../TestLabels.cs
    ../TestLeak.cs
    ../TestManagedHash.cs
    ../TestMarshalling.cs
    ../TestMarshallingOverflow.cs
    ../TestModelLoading.cs
    ../TestMultiClassPredictionFactory.cs
    ../TestMultiworldTesting.cs
    ../TestNull.cs
    ../TestPooling.cs
    ../TestRunner.cs
    ../TestSearch.cs
    ../TestSerializer.cs
    ../TestTelemetryProcessor.cs
    ../TestTracing.cs
    ../TestWiki.cs
    ../TestWrapper.cs
    ../TrainSet0002Dat.cs
    ../TrainSetCs_testLdf.cs
    ../VWTestHelper.cs
    ../VowpalWabbitExampleJsonValidator.cs
    ../VowpalWabbitExampleValidator.cs
    ../VowpalWabbitListenerToEvents.cs
    ../VowpalWabbitStdErrPerformanceStatistics.cs
)

if (vw_DOTNET_USE_MSPROJECT)
  include_external_msproject(VW.Net.UnitTest ${CMAKE_CURRENT_SOURCE_DIR}/vw.unittest.csproj VowpalWabbit VowpalWabbit.JSON VowpalWabbit.Parallel console.net testcommon)
else()
  add_custom_target(VW.Net.UnitTest ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:vw.net.native> --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>" -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building VW.Net.UnitTest
    DEPENDS VowpalWabbit VowpalWabbit.JSON VowpalWabbit.Parallel console.net testcommon
    SOURCES ${VW_NET_UNITTEST_SOURCES})
  set_property(TARGET VW.Net.UnitTest APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_LIST_DIR}/obj")
endif()

if (BUILD_TESTING)
  # C# tests expect files from top level test/ directory to be copied to the binary output directory
  add_custom_target(VW.Net.UnitTest.CopyTestFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/test ${PROJECT_SOURCE_DIR}/cs/unittest/json $<TARGET_FILE_DIR:vw.net.native>/test
    COMMENT "Copying test files to C# binary directory..."
  )

  # TODO: Enable TRX test logging
  add_test(
    NAME VW.Net.UnitTest.CopyTestFilesBeforeTest
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target VW.Net.UnitTest.CopyTestFiles
  )
  add_test(
    NAME VW.Net.UnitTest
    COMMAND ${DOTNET_COMMAND} test $<TARGET_FILE_DIR:vw.net.native>/VW.Net.UnitTest.dll --Platform:x64 --InIsolation --logger "console;verbosity=detailed"
  )
  if (VW_USE_ASAN AND NOT WIN32)
    # If using AddressSanitizer on non-Windows platform, libasan must be preloaded
    # Ignore alloc-dealloc mismatch since objects may be allocated in C++ and freed in C# for example
    # Ignore leaks because it detects stuff from .NET itself
    set_tests_properties(
      VW.Net.UnitTest PROPERTIES
      ENVIRONMENT "LD_PRELOAD=libasan.so.6;ASAN_OPTIONS=alloc_dealloc_mismatch=0,detect_leaks=0"
    )
  endif()
  set_tests_properties(VW.Net.UnitTest PROPERTIES DEPENDS VW.Net.UnitTest.CopyTestFilesBeforeTest)
endif()
