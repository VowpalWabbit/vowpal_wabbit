param([string]$apiKey="", [string]$outputDirectory="..\..\vowpalwabbit\x64\Release")

$version = Get-Content version.txt

..\..\vowpalwabbit\packages\gitlink.2.2.0\lib\net45\GitLink.exe ..\.. -f vowpalwabbit\vw.sln -p x64 -c Release -d ..\..\vowpalwabbit\x64\Release
..\..\vowpalwabbit\.nuget\nuget pack cs.nuspec -Version $version -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\..\vowpalwabbit\ -OutputDirectory $outputDirectory
..\..\vowpalwabbit\.nuget\nuget pack cs-json.nuspec -Version $version -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\..\vowpalwabbit\ -OutputDirectory $outputDirectory
..\..\vowpalwabbit\.nuget\nuget pack cs-parallel.nuspec -Version $version -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\..\vowpalwabbit\ -OutputDirectory $outputDirectory


if (not [string]::IsNullOrEmpty($apiKey))
{ 
    ..\..\vowpalwabbit\.nuget\nuget push ..\..\vowpalwabbit\x64\Release\VowpalWabbit.*.nupkg $apiKey
}
