REM Integration points for toolchain customization
IF NOT DEFINED nugetPath (
    SET nugetPath=nuget
)

IF NOT DEFINED msbuildPath (
    SET msbuildPath=msbuild
)

REM Repo-specific paths
IF NOT DEFINED vwRoot (
    SET vwRoot=%~dp0..
)