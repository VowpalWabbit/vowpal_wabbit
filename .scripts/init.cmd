REM Integration points for toolchain customization
IF NOT DEFINED nugetPath (
    SET nugetPath=nuget
)

IF NOT DEFINED msbuildPath (
    CALL %~dp0find-vs2017.cmd
)

IF NOT DEFINED msbuildPath (
    IF NOT EXIST "%VsInstallDir%\MSBuild\15.0\Bin\MSBuild.exe" (
        ECHO ERROR: MsBuild couldn't be found
        EXIT /b 1
    ) ELSE (
        SET "msBuildPath=%VsInstallDir%\MSBuild\15.0\Bin\MSBuild.exe"
    )
)

REM Repo-specific paths
IF NOT DEFINED vwRoot (
    SET vwRoot=%~dp0..
)