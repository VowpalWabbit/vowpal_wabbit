REM Integration points for toolchain customization
IF NOT DEFINED nugetPath (
    SET nugetPath=nuget
)

IF NOT DEFINED msbuildPath (
    CALL %~dp0find-vs2017.cmd
)

IF NOT DEFINED vstestPath (
    CALL %~dp0find-vs2017.cmd
)

IF NOT DEFINED msbuildPath (
    IF EXIST "%VsInstallDir%\MSBuild\15.0\Bin\MSBuild.exe" (
        SET "msBuildPath=%VsInstallDir%\MSBuild\15.0\Bin\MSBuild.exe"
    ) ELSE (
        IF EXIST "%VsInstallDir%\MSBuild\Current\Bin\MSBuild.exe" (
            SET "msBuildPath=%VsInstallDir%\MSBuild\Current\Bin\MSBuild.exe"
        ) ELSE (
            ECHO Failed to find vstest.console.exe in "%VsInstallDir%\MSBuild\Current\Bin\MSBuild.exe"
            ECHO ERROR: MsBuild couldn't be found
            EXIT /b 1
        )
    )
)

IF NOT DEFINED vstestPath (
    IF EXIST "%VsInstallDir%\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe" (
        SET "vstestPath=%VsInstallDir%\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe"
    ) ELSE (
        ECHO Failed to find vstest.console.exe in "%VsInstallDir%\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe"
        ECHO ERROR: vstest.console couldn't be found
        EXIT /b 1
    )
)

IF NOT DEFINED vcpkgPath (
    IF NOT DEFINED VCPKG_INSTALLATION_ROOT (
        ECHO ERROR: vcpkgPath is not configured, but VCPKG_INSTALLATION_ROOT is also not configured. Cannot find vcpkg.
        EXIT /b 1
    )

    SET "vcpkgPath=C:\vcpkg\vcpkg.exe"
    SET "VcpkgIntegration=C:\vcpkg\scripts\buildsystems\msbuild\vcpkg.targets"
    SET "VcpkgProperties=C:\vcpkg\scripts\buildsystems\msbuild\vcpkg.props"
)

IF NOT DEFINED flatcPath (
    IF NOT DEFINED VCPKG_INSTALLATION_ROOT (
        ECHO ERROR: flatcPath is not configured, but VCPKG_INSTALLATION_ROOT is also not configured. Cannot find vcpkg.
        EXIT /b 1
    )

    SET "flatcPath=C:\vcpkg\installed\x64-windows\tools\flatbuffers\flatc.exe"
)

echo %VCPKG_INSTALLATION_ROOT%

REM Repo-specific paths
IF NOT DEFINED vwRoot (
    SET vwRoot=%~dp0..
)