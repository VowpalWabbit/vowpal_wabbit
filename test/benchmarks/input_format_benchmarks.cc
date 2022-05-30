#include "benchmarks_common.h"
#include "vw/core/cache.h"
#include "vw/core/parse_example.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"

#include <benchmark/benchmark.h>

#include <array>
#include <cstdio>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

std::shared_ptr<std::vector<char>> get_cache_buffer(const std::string& es)
{
  auto* vw = VW::initialize("--cb 2 --quiet");
  auto buffer = std::make_shared<std::vector<char>>();
  vw->example_parser->output.add_file(VW::io::create_vector_writer(buffer));
  auto* ae = &VW::get_unused_example(vw);
  VW::read_line(*vw, ae, const_cast<char*>(es.c_str()));

  VW::details::cache_temp_buffer temp_buf;
  VW::write_example_to_cache(vw->example_parser->output, ae, vw->example_parser->lbl_parser, vw->parse_mask, temp_buf);
  vw->example_parser->output.flush();
  VW::finish_example(*vw, *ae);
  VW::finish(*vw);

  return buffer;
}

template <class... ExtraArgs>
static void bench_cache_io_buf(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::array<std::string, sizeof...(extra_args)> res = {extra_args...};
  auto example_string = res[0];

  auto cache_buffer = get_cache_buffer(example_string);
  auto* vw = VW::initialize("--cb 2 --quiet");
  io_buf io_buffer;
  io_buffer.add_file(VW::io::create_buffer_view(cache_buffer->data(), cache_buffer->size()));
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw));

  for (auto _ : state)
  {
    VW::read_example_from_cache(vw, io_buffer, examples);
    VW::empty_example(*vw, *examples[0]);
    io_buffer.reset();
    benchmark::ClobberMemory();
  }
  VW::finish(*vw);
}

template <class... ExtraArgs>
static void bench_text_io_buf(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::array<std::string, sizeof...(extra_args)> res = {extra_args...};
  auto example_string = res[0];

  auto* vw = VW::initialize("--cb 2 --quiet");
  VW::multi_ex examples;
  io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  examples.push_back(&VW::get_unused_example(vw));

  for (auto _ : state)
  {
    vw->example_parser->reader(vw, buffer, examples);
    VW::empty_example(*vw, *examples[0]);
    buffer.reset();
    benchmark::ClobberMemory();
  }
  VW::finish(*vw);
}

static void benchmark_example_reuse(benchmark::State& state)
{
  std::string example_string =
      "1 1.0 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures "
      "NumberOfLegs:4.0 HasStripes";

  auto* vw = VW::initialize("--quiet", nullptr, false, nullptr, nullptr);
  io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  for (auto _ : state)
  {
    examples.push_back(&VW::get_unused_example(vw));
    vw->example_parser->reader(vw, buffer, examples);
    VW::finish_example(*vw, *examples[0]);
    buffer.reset();
    examples.clear();
    benchmark::ClobberMemory();
  }
  VW::finish(*vw);
}

BENCHMARK_CAPTURE(bench_cache_io_buf, 120_string_fts, get_x_string_fts(120));
BENCHMARK_CAPTURE(bench_text_io_buf, 120_string_fts, get_x_string_fts(120));

BENCHMARK_CAPTURE(bench_cache_io_buf, 120_num_fts, get_x_numerical_fts(120));
BENCHMARK_CAPTURE(bench_text_io_buf, 120_num_fts, get_x_numerical_fts(120));

BENCHMARK(benchmark_example_reuse);
