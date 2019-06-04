IF NOT DEFINED VsInstallDir (
    IF NOT EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        ECHO Failed to find VsWhere. Unable to discover VS2017 Installation Path.
        EXIT /B 1
    )
    
    REM Try to find VS Install
    FOR /f "usebackq tokens=*" %%i IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) DO (
        SET "VsInstallDir=%%i"
    )
)