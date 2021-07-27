#include <benchmark/benchmark.h>
#include "vw_string_view.h"

struct by_ref
{
  size_t save(VW::string_view& ref)
  {
    _save = ref;
    return _save.size();
  }

  VW::string_view& get() { return _save; }
  VW::string_view _save;
};

struct by_val
{
  size_t save(VW::string_view val)
  {
    _save = val;
    return _save.size();
  }

  VW::string_view get() { return _save; }
  VW::string_view _save;
};

template <typename T>
static void string_view(benchmark::State& state)
{
  T r;
  VW::string_view sv("some data");

  for (auto _ : state)
  {
    benchmark::DoNotOptimize(r.save(sv));
    benchmark::DoNotOptimize(r.get());
    benchmark::ClobberMemory();
  }
}

static void string_view_by_ref(benchmark::State& state) { string_view<by_ref>(state); }

static void string_view_by_val(benchmark::State& state) { string_view<by_val>(state); }

BENCHMARK(string_view_by_val);
BENCHMARK(string_view_by_ref);
