..\..\vowpalwabbit\packages\gitlink.2.2.0\lib\net45\GitLink.exe ..\.. -f vowpalwabbit\vw.sln -p x64 -c Release -d ..\..\vowpalwabbit\x64\Release
..\..\vowpalwabbit\.nuget\nuget pack cs.nuspec -Prop "Configuration=Release;Platform=x64" -Prop SolutionDir=..\..\vowpalwabbit\ -OutputDirectory ..\..\vowpalwabbit\x64\Release

..\..\vowpalwabbit\.nuget\nuget push ..\..\vowpalwabbit\x64\Release\VowpalWabbit.*.nupkg %NUGETAPIKEY%
rem ignore nuget push error
set errorlevel=
