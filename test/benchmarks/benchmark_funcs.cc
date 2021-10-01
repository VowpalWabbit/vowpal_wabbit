#include <benchmark/benchmark.h>

#include <string>

#include "parser.h"
#include "io/io_adapter.h"
#include "vw.h"

static void benchmark_sum_ft_squared_char(benchmark::State& state)
{
  std::string example_string =
      "1 1.0 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures "
      "NumberOfLegs:4.0 HasStripes";

  auto vw = VW::initialize("--quiet -q MS --cubic MOS", nullptr, false, nullptr, nullptr);

  v_array<example*> examples;
  io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  examples.push_back(&VW::get_unused_example(vw));
  vw->example_parser->reader(vw, buffer, examples);
  example* ex = examples[0];
  VW::setup_example(*vw, ex);
  for (auto _ : state)
  {
    auto result = ex->get_total_sum_feat_sq();
    ex->reset_total_sum_feat_sq();
    benchmark::DoNotOptimize(result);
  }
  VW::finish_example(*vw, *ex);
  VW::finish(*vw);
}

static void benchmark_sum_ft_squared_extent(benchmark::State& state)
{
  std::string example_string =
      "1 1.0 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures "
      "NumberOfLegs:4.0 HasStripes";

  auto vw = VW::initialize("--quiet --new_full_interactions MetricFeatures|Says --new_full_interactions MetricFeatures|OtherFeatures|Says", nullptr, false, nullptr, nullptr);

  v_array<example*> examples;
  io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  examples.push_back(&VW::get_unused_example(vw));
  vw->example_parser->reader(vw, buffer, examples);
  example* ex = examples[0];
  VW::setup_example(*vw, ex);
  for (auto _ : state)
  {
    auto result = ex->get_total_sum_feat_sq();
    ex->reset_total_sum_feat_sq();
    benchmark::DoNotOptimize(result);
  }
  VW::finish_example(*vw, *ex);
  VW::finish(*vw);
}

BENCHMARK(benchmark_sum_ft_squared_char);
BENCHMARK(benchmark_sum_ft_squared_extent);
