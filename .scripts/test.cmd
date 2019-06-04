@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

REM TODO: Ensure all errors caught by the smoke test will result in non-zero exit codes
ECHO Running VW C Smoke test
"%vwRoot%\vowpalwabbit\out\target\Release\x64\c_test.exe"

IF "%ERRORLEVEL%" NEQ "0" (
    ENDLOCAL
    EXIT /B %ERRORLEVEL%
)

ECHO Running VW Unit Tests in C++
"%vwRoot%\vowpalwabbit\out\target\Release\x64\vw_unit_test.exe"

IF "%ERRORLEVEL%" NEQ "0" (
    ENDLOCAL
    EXIT /B %ERRORLEVEL%
)

ECHO Running VW Unit Tests in C#
"%vstestPath%" /Platform:x64 /inIsolation "%vwRoot%\vowpalwabbit\x64\Release\cs_unittest.dll" /TestCaseFilter:"TestCategory!=NotOnVSO" --logger:trx "--ResultsDirectory:%vwRoot%\vowpalwabbit\out\test\Release\x64"

IF "%ERRORLEVEL%" NEQ "0" (
    ENDLOCAL
    EXIT /B %ERRORLEVEL%
)

ECHO Running C# Exploration Library Tests...
REM These really should be using some standard harness so we can get output logs into CI systems
"%vwRoot%\cs\explore\bin\Release\ds_explore_cs_test.exe" "%vwRoot%\test\explore\test1-input.txt" "%vwRoot%\test\explore\test1-expected.txt"

IF "%ERRORLEVEL%" NEQ "0" (
    ECHO Failed.
    ENDLOCAL
    EXIT /B %ERRORLEVEL%
)

ENDLOCAL