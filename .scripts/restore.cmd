@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

REM TODO: This really should be out-of-source
%VCPKG_INSTALLATION_ROOT%\vcpkg install flatbuffers:x64-windows boost-test:x64-windows

ENDLOCAL
