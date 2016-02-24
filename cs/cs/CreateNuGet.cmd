..\..\vowpalwabbit\.nuget\nuget pack cs.nuspec -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\..\vowpalwabbit\ -OutputDirectory ..\..\vowpalwabbit\x64\Release

..\..\vowpalwabbit\.nuget\nuget push ..\..\vowpalwabbit\x64\Release\VowpalWabbit.*.nupkg %NUGETAPIKEY%
rem ignore nuget push error
set errorlevel=
