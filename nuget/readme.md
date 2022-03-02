# Native Nuget Packages

There are native nuget packages available for the core VW libraries and the main command line executable. They can be found as artifacts produced by this [CI job](https://github.com/VowpalWabbit/vowpal_wabbit/actions/workflows/build_nugets.yml).

There is a package available per toolset version. Currently supported:
- v141
- v142

Details of Nuget packages:
- amd64 only
- Static library only (VW does not currently support dynamic linkage)
- Targeting the multi-threaded dynamically linked runtime
- Contains artifacts for debug and release mode builds
- Contains the necessary dependent static libs and headers (fmt, spdlog, zlib, rapidjson)

How to use:
1. Install the nuget to your package directory
2. Include the `.targets` file into your project

For example:

`myproj.vcxproj`:
```xml
<ImportGroup Label="ExtensionTargets">
    <Import Project="$(ProjectDir)packages\VowpalWabbitNative-v142.9.0.0\build\vowpalwabbit.targets" Condition="Exists('$(ProjectDir)packages\VowpalWabbitNative-v142.9.0.0\build\vowpalwabbit.targets')" />
</ImportGroup>
```