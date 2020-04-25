@echo off

IF %1.==. GOTO USAGE

pushd %1
set VCPKG_PATH=%CD%
popd

cmake %~dp0 -G "Visual Studio 14 2015 Win64" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%\scripts\buildsystems\vcpkg.cmake
msbuild vowpal_wabbit.sln /p:Configuration=Release
GOTO:EOF

:USAGE
    ECHO Usage: build-windows-experimental.bat [vcpkg_rootdir]
    GOTO:EOF