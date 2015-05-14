@echo off
rem some variable mixup is blocking this to be part of VS
..\vowpalwabbit\.nuget\nuget pack cs.csproj -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\vowpalwabbit\ -OutputDirectory bin\x64\Release
