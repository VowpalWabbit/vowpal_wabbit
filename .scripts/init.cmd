REM Integration points for toolchain customization
IF NOT DEFINED nugetPath (
    SET "nugetPath=nuget"
)

IF NOT DEFINED msbuildPath (
    REM Try to find VS Install
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
        set "InstallDir=%%i"
    )

    if not exist "%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe" (
        echo ERROR: MsBuild couldn't be found
        exit /b 1
    ) else (
        SET "msBuildPath=%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe"
    )
)

REM Repo-specific paths
IF NOT DEFINED vwRoot (
    SET vwRoot=%~dp0..
)
