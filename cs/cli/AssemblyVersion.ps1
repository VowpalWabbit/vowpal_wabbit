param([string]$projectDir)

$scriptDir = Split-Path -parent $PSCommandPath
$version = Get-Content "$projectDir\..\nuget\version.txt"

$cpp = Get-Content "$projectDir\AssemblyInfo.cpp"
$cpp = $cpp -replace "Version\(""[0-9\.]+""\)", "Version(""$version"")"
$cpp = $cpp -replace "VersionAttribute\(""[0-9\.]+""\)", "VersionAttribute(""$version"")"
Set-Content -Path "$projectDir\AssemblyInfo.cpp" -Value $cpp

$rc = Get-Content "$projectDir\Resource.rc"
$rc = $rc -replace "FileVersion"", ""[0-9,\.]+""", "FileVersion"", ""$version"""
$rc = $rc -replace "ProductVersion"", ""[0-9,\.]+""", "ProductVersion"", ""$version"""

$version = $version -replace "\.", ","
$rc = $rc -replace "FILEVERSION [0-9,]+", "FILEVERSION $version"
$rc = $rc -replace "PRODUCTVERSION [0-9,]+", "PRODUCTVERSION $version"

Set-Content -Path "$projectDir\Resource.rc" -Value $rc
