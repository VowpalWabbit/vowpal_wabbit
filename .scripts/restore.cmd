@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

REM CD out of the repo dir as we need to avoid vcpkg recognizing the manifest
cd ..

REM TODO: This really should be out-of-source
%VCPKG_INSTALLATION_ROOT%\vcpkg install flatbuffers:x64-windows

ENDLOCAL
