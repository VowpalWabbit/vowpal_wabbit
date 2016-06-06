$version = Get-Content version.txt

function PatchAssemblyInfo ($file)
{
    $text = (Get-Content $file) -replace "Version\(""[0-9\.]+""\)", "Version(""$version"")"
    Set-Content -Path $file -Value $text
}

PatchAssemblyInfo "..\cs\Properties\AssemblyInfo.cs"
PatchAssemblyInfo "..\cs_json\Properties\AssemblyInfo.cs"
PatchAssemblyInfo "..\cs_parallel\Properties\AssemblyInfo.cs"
PatchAssemblyInfo "..\common\Properties\AssemblyInfo.cs"
PatchAssemblyInfo "..\cli\AssemblyInfo.cpp"

$rc = Get-Content "..\cli\Resource.rc"
$rc = $rc -replace "FileVersion"", ""[0-9,\.]+""", "FileVersion"", ""$version"""
$rc = $rc -replace "ProductVersion"", ""[0-9,\.]+""", "ProductVersion"", ""$version"""

$version = $version -replace "\.", ","
$rc = $rc -replace "FILEVERSION [0-9,]+", "FILEVERSION $version"
$rc = $rc -replace "PRODUCTVERSION [0-9,]+", "PRODUCTVERSION $version"

Set-Content -Path "..\cli\Resource.rc" -Value $rc
