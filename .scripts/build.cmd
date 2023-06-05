@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

REM TODO: Figure out how to parametrize this script?! (is there a standard, or do we actually need parse args?)
ECHO Building "%vwRoot%" for Release x64

REM CMAKE_PROGRAM_PATH is for nuget and texttransform
cmake -S "%vwRoot%" -B "%vwRoot%\build" -G "Visual Studio 16 2019" -A "x64" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_INSTALLATION_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DVCPKG_MANIFEST_MODE=OFF ^
    -Dvw_BUILD_NET_FRAMEWORK=On ^
    -DVW_FEAT_FLATBUFFERS=On ^
    -DVW_FEAT_CSV=On ^
    -DVW_FEAT_CB_GRAPH_FEEDBACK=On ^
    -Dvw_BUILD_NET_FRAMEWORK=On ^
    -DRAPIDJSON_SYS_DEP=Off ^
    -DFMT_SYS_DEP=Off ^
    -DSPDLOG_SYS_DEP=OFF ^
    -DVW_ZLIB_SYS_DEP=OFF ^
    -DVW_BOOST_MATH_SYS_DEP=OFF ^
    -DVW_BUILD_VW_C_WRAPPER=Off ^
    "-DCMAKE_PROGRAM_PATH=%vwRoot%\vowpalwabbit\.nuget;%VsInstallDir%\Common7\IDE;%VCPKG_INSTALLATION_ROOT%\installed\x64-windows\tools\flatbuffers;%VsInstallDir%\Common7\IDE\CommonExtensions\Microsoft\TestWindow"

cmake --build "%vwRoot%\build" --config Release

POPD

ENDLOCAL
