#include "vw/common/future_compat.h"

#include <benchmark/benchmark.h>

#include <memory>

void pointer_create_raw(benchmark::State& state)
{
  for (auto _ : state)
  {
    int* x = new int{0};
    benchmark::DoNotOptimize(x);
    delete x;
    benchmark::ClobberMemory();
  }
}

void pointer_create_unique(benchmark::State& state)
{
  for (auto _ : state)
  {
    std::unique_ptr<int> x(new int{0});
    benchmark::ClobberMemory();
  }
}

void pointer_create_shared(benchmark::State& state)
{
  for (auto _ : state)
  {
    std::shared_ptr<int> x(new int{0});
    benchmark::ClobberMemory();
  }
}

#ifdef HAS_STD14
void pointer_create_make_unique(benchmark::State& state)
{
  for (auto _ : state)
  {
    auto x = std::make_unique<int>(0);
    benchmark::ClobberMemory();
  }
}
#endif

void pointer_create_make_shared(benchmark::State& state)
{
  for (auto _ : state)
  {
    auto x = std::make_shared<int>(0);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(pointer_create_raw);
BENCHMARK(pointer_create_unique);
BENCHMARK(pointer_create_shared);
#ifdef HAS_STD14
BENCHMARK(pointer_create_make_unique);
#endif
BENCHMARK(pointer_create_make_shared);

void pointer_copy_raw(benchmark::State& state)
{
  int* x = new int{0};
  for (auto _ : state)
  {
    auto y = x;
    benchmark::DoNotOptimize(y);
    benchmark::ClobberMemory();
  }
  delete x;
}

void pointer_copy_shared(benchmark::State& state)
{
  auto x = std::make_shared<int>(0);
  for (auto _ : state)
  {
    auto y = x;
    benchmark::DoNotOptimize(y);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(pointer_copy_raw);
BENCHMARK(pointer_copy_shared);

void pointer_access_raw(benchmark::State& state)
{
  int* x = new int{0};
  for (auto _ : state)
  {
    *x += 1;
    benchmark::ClobberMemory();
  }
  delete x;
}

void pointer_access_unique(benchmark::State& state)
{
  std::unique_ptr<int> x(new int{0});
  for (auto _ : state)
  {
    *x += 1;
    benchmark::ClobberMemory();
  }
}

void pointer_access_shared(benchmark::State& state)
{
  std::shared_ptr<int> x(new int{0});
  for (auto _ : state)
  {
    *x += 1;
    benchmark::ClobberMemory();
  }
}

BENCHMARK(pointer_access_raw);
BENCHMARK(pointer_access_unique);
BENCHMARK(pointer_access_shared);

/*
---------------------------------------------------------------------
Benchmark                           Time             CPU   Iterations
---------------------------------------------------------------------
pointer_create_raw               15.0 ns         15.0 ns     44357700
pointer_create_unique            14.8 ns         14.8 ns     45669518
pointer_create_shared            34.2 ns         34.2 ns     20097392
pointer_create_make_unique       14.8 ns         14.8 ns     46414020
pointer_create_make_shared       19.0 ns         19.0 ns     36023500
pointer_copy_raw                0.330 ns        0.330 ns   1000000000
pointer_copy_shared              3.32 ns         3.32 ns    210186414
pointer_access_raw               1.79 ns         1.79 ns    390971464
pointer_access_unique            1.79 ns         1.79 ns    373624462
pointer_access_shared            1.78 ns         1.78 ns    394688841
*/
