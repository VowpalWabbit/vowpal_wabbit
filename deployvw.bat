@echo off
if "%1" == "" goto usage
if not "%2" == "" goto usage
if "%VSINSTALLDIR%" == "" goto missing
if /i %1 == x86       goto x86
if /i %1 == x64       goto x64


if %VSINSTALLDIR%

:x86
devenv vowpalwabbit\vw.sln /Clean "Release|x86"
devenv vowpalwabbit\vw.sln /Rebuild "Release|x86"
xcopy /v /i /r  /y  vowpalwabbit\x86\Release\vw.exe   deploy\x86\Release\
xcopy /v /i /r  /y  vowpalwabbit\x86\Release\spanning_tree.exe   deploy\x86\Release\
xcopy /v /i /r  /y "%VSINSTALLDIR%VC\redist\x86\Microsoft.VC120.CRT" deploy\x86\Release\
xcopy /v /i /r  /y "%VSINSTALLDIR%VC\redist\x86\Microsoft.VC120.CXXAMP" deploy\x86\Release\
xcopy /v /i /r  /y "%VSINSTALLDIR%VC\redist\x86\Microsoft.VC120.OPENMP" deploy\x86\Release\
goto :eof

:x64
devenv vowpalwabbit\vw.sln /Clean "Release|x64"
devenv vowpalwabbit\vw.sln /Rebuild "Release|x64"
xcopy /v /i /r  /y vowpalwabbit\x64\Release\vw.exe   deploy\x64\Release\
xcopy /v /i /r  /y vowpalwabbit\x64\Release\spanning_tree.exe   deploy\x64\Release\
xcopy /v /i /r  /y "%VSINSTALLDIR%VC\redist\x64\Microsoft.VC120.CRT" deploy\x64\Release\
xcopy /v /i /r  /y "%VSINSTALLDIR%VC\redist\x64\Microsoft.VC120.CXXAMP" deploy\x64\Release\
xcopy /v /i /r  /y "%VSINSTALLDIR%VC\redist\x64\Microsoft.VC120.OPENMP" deploy\x64\Release\
goto :eof


:usage
echo Error in script usage. The correct usage is:
echo     %0 [option]
echo where [option] is: x86 ^| x64
echo:
echo For example:
echo     %0 x86
goto :eof

:missing
echo The variable "VSINSTALLDIR"  is missing.  Visual Studio 2013 might not be installed.
goto :eof

:eof
