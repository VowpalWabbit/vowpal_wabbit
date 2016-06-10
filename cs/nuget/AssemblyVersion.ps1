param([string]$projectDir)

$scriptDir = Split-Path -parent $PSCommandPath
$assemblyInfo = "$projectDir\Properties\AssemblyInfo.cs"
$version = Get-Content "$scriptDir\version.txt"

$text = (Get-Content $assemblyInfo) -replace "Version\(""[0-9\.]+""\)", "Version(""$version"")"
Set-Content -Path $assemblyInfo -Value $text
