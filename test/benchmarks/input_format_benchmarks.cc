#include <benchmark/benchmark.h>

#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <memory>
#include <array>
#include <cstdio>
#include <random>
#include <unordered_map>

#include "cache.h"
#include "parser.h"
#include "io/io_adapter.h"
#include "vw.h"
#include "benchmarks_common.h"

std::shared_ptr<std::vector<char>> get_cache_buffer(const std::string& es)
{
  auto vw = VW::initialize("--cb 2 --quiet");
  auto buffer = std::make_shared<std::vector<char>>();
  vw->example_parser->output.add_file(VW::io::create_vector_writer(buffer));
  vw->example_parser->write_cache = true;
  auto ae = &VW::get_unused_example(vw);

  VW::read_line(*vw, ae, const_cast<char*>(es.c_str()));

  if (vw->example_parser->write_cache)
  {
    vw->example_parser->lbl_parser.cache_label(&ae->l, ae->_reduction_features, vw->example_parser->output);
    cache_features(vw->example_parser->output, ae, vw->parse_mask);
  }
  vw->example_parser->output.flush();
  VW::finish_example(*vw, *ae);
  VW::finish(*vw);

  return buffer;
}

template <class... ExtraArgs>
static void bench_cache_io_buf(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::string res[sizeof...(extra_args)] = {extra_args...};
  auto example_string = res[0];

  auto buffer = get_cache_buffer(example_string);
  auto vw = VW::initialize("--cb 2 --quiet");

  v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(vw));

  for (auto _ : state)
  {
    vw->example_parser->input.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size()));
    read_cached_features(vw, examples);
    VW::empty_example(*vw, *examples[0]);
    benchmark::ClobberMemory();
  }
  VW::finish(*vw);
}

template <class... ExtraArgs>
static void bench_cache_io_buf_collections(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::string res[sizeof...(extra_args)] = {extra_args...};
  auto example_string = res[0];
  auto examples_size = std::stoi(res[1]);

  auto buffer = get_cache_buffer(example_string);
  auto vw = VW::initialize("--cb 2 --quiet");

  v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(vw));

  for (auto _ : state)
  {
    for (size_t i = 0; i < examples_size; i++)
    { vw->example_parser->input.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size())); }
    while (read_cached_features(vw, examples)) { VW::empty_example(*vw, *examples[0]); }
    benchmark::ClobberMemory();
  }
  VW::finish(*vw);
}

template <class... ExtraArgs>
static void bench_text_io_buf(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::string res[sizeof...(extra_args)] = {extra_args...};
  auto example_string = res[0];

  auto vw = VW::initialize("--cb 2 --quiet");
  v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(vw));

  for (auto _ : state)
  {
    vw->example_parser->input.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
    vw->example_parser->reader(vw, examples);
    VW::empty_example(*vw, *examples[0]);
    benchmark::ClobberMemory();
  }
  VW::finish(*vw);
}

static void benchmark_example_reuse(benchmark::State& state)
{
  std::string example_string =
      "1 1.0 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures "
      "NumberOfLegs:4.0 HasStripes";

  auto vw = VW::initialize("--quiet", nullptr, false, nullptr, nullptr);

  v_array<example*> examples;

  for (auto _ : state)
  {
    examples.push_back(&VW::get_unused_example(vw));
    vw->example_parser->input.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
    vw->example_parser->reader(vw, examples);
    VW::finish_example(*vw, *examples[0]);
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
