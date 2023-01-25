#include <benchmark/benchmark.h>
#include "vw/common/future_compat.h"

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
