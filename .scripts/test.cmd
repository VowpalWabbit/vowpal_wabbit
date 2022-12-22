@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

ECHO Running VW Unit Tests in C#
REM TODO: Add explicit logging configuration so it can be uploaded to pipeline results.
"%vstestPath%" /Platform:x64 /inIsolation "%vwRoot%\build\binaries\Release\cs_unittest.dll" /TestCaseFilter:"TestCategory!=NotOnVSO" --logger:trx "--ResultsDirectory:%vwRoot%\vowpalwabbit\out\test\Release\x64"

IF "%ERRORLEVEL%" NEQ "0" (
    ENDLOCAL
    EXIT /B %ERRORLEVEL%
)

ENDLOCAL