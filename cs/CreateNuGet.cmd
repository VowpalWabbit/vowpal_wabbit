..\vowpalwabbit\.nuget\nuget pack cs.nuspec -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\vowpalwabbit\ -OutputDirectory bin\x64\Release

..\vowpalwabbit\.nuget\nuget push bin\x64\Release\VowpalWabbit.*.nupkg %NUGETAPIKEY%
