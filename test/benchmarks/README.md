# VW Benchmarks

## Benchmark results
Automated benchmarking is performed as part of the VW continuous integration system. Benchmark results are available here: **https://vowpalwabbit.org/vowpal_wabbit/dev/bench/**

Benchmarking is done via [github-action-benchmark](https://github.com/benchmark-action/github-action-benchmark).

## Running benchmarks locally
### C++
Compile:
```
cmake --preset vcpkg-release -DBUILD_BENCHMARKS=On
cmake --build build --target vw-benchmarks.out
```

Run:
```
./build/test/benchmarks/vw-benchmarks.out
```

### .NET
First, install the VW Nuget packages.

If you want graphs of benchmark results to be automatically generated, make sure that the `RScript` executable is installed and on your PATH.
```sh
cd test/benchmarks/dotnet

# Build for both .NET Core and Framework
dotnet build dotnet_benchmark.csproj --framework net6.0 --runtime win-x64 --configuration Release --no-restore --self-contained
dotnet build dotnet_benchmark.csproj --framework net4.8 --runtime win-x64 --configuration Release --no-restore --self-contained

# Run with .NET Core as host process
# Arguments after "--" are for BenchmarkDotNet
dotnet run --project dotnet_benchmark.csproj --framework net6.0 --runtime win-x64 --configuration Release --no-build -- --filter '*' --join

# Go to output directory
cd BenchmarkDotNet.Artifacts/results
```
