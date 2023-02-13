#include "../../vowpalwabbit/core/tests/simulator.h"
#include "benchmarks_common.h"
#include "vw/cache_parser/parse_example_cache.h"
#include "vw/config/options_cli.h"
#include "vw/core/learner.h"
#include "vw/core/metric_sink.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/epsilon_decay.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/text_parser/parse_example_text.h"

#include <benchmark/benchmark.h>

template <class... ExtraArgs>
static void bench_epsilon_decay(benchmark::State& state, bool use_decay, ExtraArgs&&... extra_args)
{
  std::array<std::string, sizeof...(extra_args)> res = {extra_args...};
  std::string model_count = res[0];
  std::string bit_size = res[1];
  std::string tolerance = res[2];

  using callback_map =
      typename std::map<size_t, std::function<bool(simulator::cb_sim&, VW::workspace&, VW::multi_ex&)>>;
  callback_map test_hooks;

  for (auto _ : state)
  {
    const size_t num_iterations = 1000;
    const size_t seed = 99;
    const std::vector<uint64_t> swap_after = {500};
    if (use_decay)
    {
      simulator::_test_helper_hook(
          std::vector<std::string>{"-l", "1e-3", "--power_t", "0", "-q::", "--cb_explore_adf", "--epsilon_decay",
              "--model_count", model_count, "-b", bit_size, "--tol_x", tolerance, "--quiet"},
          test_hooks, num_iterations, seed, swap_after);
    }
    else
    {
      simulator::_test_helper_hook(
          std::vector<std::string>{"-l", "1e-3", "--power_t", "0", "-q::", "--cb_explore_adf", "--quiet"}, test_hooks,
          num_iterations, seed, swap_after);
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_CAPTURE(bench_epsilon_decay, epsilon_decay_1_model_big_tol, true, "1", "18", "1e-2");
BENCHMARK_CAPTURE(bench_epsilon_decay, epsilon_decay_2_model_big_tol, true, "2", "19", "1e-2");
BENCHMARK_CAPTURE(bench_epsilon_decay, epsilon_decay_4_model_big_tol, true, "4", "20", "1e-2");
BENCHMARK_CAPTURE(bench_epsilon_decay, epsilon_decay_1_model_small_tol, true, "1", "18", "1e-6");
BENCHMARK_CAPTURE(bench_epsilon_decay, epsilon_decay_2_model_small_tol, true, "2", "19", "1e-6");
BENCHMARK_CAPTURE(bench_epsilon_decay, epsilon_decay_4_model_small_tol, true, "4", "20", "1e-6");
BENCHMARK_CAPTURE(bench_epsilon_decay, without_epsilon_decay, false, "", "", "");