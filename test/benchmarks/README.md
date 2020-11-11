install google benchmark: https://github.com/google/benchmark

by default google benchmark is built in Debug mode so you might want to specify Release mode when building/installing

build vw benchmarks:

```
make -j vw-benchmarks.out
```

run vw benchmarks:

```
./test/benchmarks/vw-benchmarks.out
```