#include "vw/config/options_cli.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"

#include <benchmark/benchmark.h>

#include <string>

static void benchmark_sum_ft_squared_char(benchmark::State& state)
{
  std::string example_string =
      "1 1.0 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures "
      "NumberOfLegs:4.0 HasStripes";

  auto vw = VW::initialize(
      VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--quiet", "-q", "MS", "--cubic", "MOS"}));

  VW::multi_ex examples;
  io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  examples.push_back(&VW::get_unused_example(vw.get()));
  vw->parser_runtime.example_parser->reader(vw.get(), buffer, examples);
  example* ex = examples[0];
  VW::setup_example(*vw, ex);
  for (auto _ : state)
  {
    auto result = ex->get_total_sum_feat_sq();
    ex->reset_total_sum_feat_sq();
    benchmark::DoNotOptimize(result);
  }
  VW::finish_example(*vw, *ex);
}

static void benchmark_sum_ft_squared_extent(benchmark::State& state)
{
  std::string example_string =
      "1 1.0 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures "
      "NumberOfLegs:4.0 HasStripes";

  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(
      std::vector<std::string>{"--quiet", "--experimental_full_name_interactions", "MetricFeatures|Says",
          "--experimental_full_name_interactions", "MetricFeatures|OtherFeatures|Says"}));

  VW::multi_ex examples;
  io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  examples.push_back(&VW::get_unused_example(vw.get()));
  vw->parser_runtime.example_parser->reader(vw.get(), buffer, examples);
  example* ex = examples[0];
  VW::setup_example(*vw, ex);
  for (auto _ : state)
  {
    auto result = ex->get_total_sum_feat_sq();
    ex->reset_total_sum_feat_sq();
    benchmark::DoNotOptimize(result);
  }
  VW::finish_example(*vw, *ex);
}

BENCHMARK(benchmark_sum_ft_squared_char);
BENCHMARK(benchmark_sum_ft_squared_extent);
