@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

REM TODO: Determine how to pass test failure out of this script so it can be used by CI/CD setups

ECHO Running VW C Smoke test
"%vwRoot%\vowpalwabbit\out\target\Release\x64\c_test.exe"

ECHO Running VW Unit Tests in C++
"%vwRoot%\vowpalwabbit\out\target\Release\x64\vw_unit_test.exe"

ECHO Running VW Unit Tests in C#
%vstestPath% /Platform:x64 /inIsolation "%vwRoot%\vowpalwabbit\x64\Release\cs_unittest.dll" /TestCaseFilter:"TestCategory!=NotOnVSO"
REM this is the same as above, just xcopied to the out folder. This is broken due to baked-in paths. The fix is to place the output straight into out.
REM %vstestPath% /Platform:x64 /inIsolation "%vwRoot%\vowpalwabbit\x64\Release\cs_unittest.dll" /TestCaseFilter:"TestCategory!=NotOnVSO"

ENDLOCAL