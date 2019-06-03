@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

REM TODO: This really should be out-of-source
ECHO Restoring "%vwRoot%\cs\cs\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\cs\cs\packages.config"
ECHO.

ECHO Restoring "%vwRoot%\cs\azure\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\cs\azure\packages.config"
ECHO.

ECHO Restoring "%vwRoot%\cs\azure_worker\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\cs\azure_worker\packages.config"
ECHO.

ECHO Restoring "%vwRoot%\cs\setup_bundle\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\cs\setup_bundle\packages.config"
ECHO.

ECHO Restoring "%vwRoot%\cs\unittest\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\cs\unittest\packages.config"
ECHO.

ECHO Restoring "%vwRoot%\python\windows27\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\python\windows27\packages.config"
ECHO.

ECHO Restoring "%vwRoot%\vowpalwabbit\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\vowpalwabbit\packages.config"
ECHO.

ECHO Restoring "%vwRoot%\cs\examples\simulator\packages.config"
"%nugetPath%" restore -o "%vwRoot%\vowpalwabbit\packages" "%vwRoot%\cs\examples\simulator\packages.config"
ECHO.

POPD

ENDLOCAL