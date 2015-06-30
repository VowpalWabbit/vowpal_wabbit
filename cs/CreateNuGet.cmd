@echo off
del bin\x64\Release\*.nupkg
..\vowpalwabbit\.nuget\nuget pack cs.nuspec -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\vowpalwabbit\ -OutputDirectory bin\x64\Release


