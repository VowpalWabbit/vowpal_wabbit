$version = Get-Content version.txt

function PatchAssemblyInfo ($file)
{
    $text = (Get-Content $file) -replace "Version\(""[0-9\.]+""\)", "Version(""$version"")"
    echo $text
    Set-Content -Path $file -Value $text
}

PatchAssemblyInfo "..\cs\Properties\AssemblyInfo.cs"
PatchAssemblyInfo "..\cs_json\Properties\AssemblyInfo.cs"
PatchAssemblyInfo "..\cs_parallel\Properties\AssemblyInfo.cs"

