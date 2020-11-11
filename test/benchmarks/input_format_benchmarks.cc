#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

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

auto get_x_numerical_fts = [](int feature_size) {
  std::stringstream ss;
  ss << "1:1:0.5 |";
  for (size_t i = 0; i < feature_size; i++) { ss << " " << std::to_string(i) + ":4.36352"; }
  std::string s = ss.str();
  return s;
};

auto get_x_string_fts = [](int feature_size) {
  std::stringstream ss;
  ss << "1:1:0.5 | ";
  for (size_t i = 0; i < feature_size; i++) { ss << "bigfeaturename" + std::to_string(i) + ":10 "; }
  std::string s = ss.str();
  return s;
};

auto get_x_string_fts_no_label = [](int feature_size, size_t action_index = 0) {
  std::stringstream ss;
  ss << " | ";
  for (size_t j = 0; j < feature_size; j++) { ss << std::to_string(action_index) + "_" + std::to_string(j) << +" "; }
  ss << std::endl;

  return ss.str();
};

auto get_x_string_fts_multi_ex = [](int feature_size, size_t actions, bool shared, bool label, size_t start_index = 0) {
  size_t action_start = 0;
  std::stringstream ss;
  if (shared) { ss << "shared | s_1 s_2 s_3 s_4" << std::endl; }
  if (label)
  {
    ss << "0:1.0:0.5 | ";
    for (size_t j = 0; j < feature_size; j++) { ss << "0_" + std::to_string(j) << +" "; }
    ss << std::endl;
    action_start++;
  }
  for (size_t i = action_start; i < actions; i++)
  {
    ss << " | ";
    for (size_t j = start_index; j < start_index + feature_size; j++)
    { ss << std::to_string(i) + "_" + std::to_string(j) << +" "; }
    ss << std::endl;
  }
  return ss.str();
};

template <class... ExtraArgs>
static void bench_text(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::string res[sizeof...(extra_args)] = {extra_args...};
  auto example_string = res[0];

  auto es = const_cast<char*>(example_string.c_str());
  auto vw = VW::initialize("--cb 2 --quiet");
  auto examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));
  for (auto _ : state)
  {
    for (size_t i = 0; i < 20000; i++)
    {
      VW::read_line(*vw, examples[0], es);
      VW::empty_example(*vw, *examples[0]);
      benchmark::ClobberMemory();
    }
  }
  examples.delete_v();
}

std::shared_ptr<std::vector<char>> get_cache_buffer(const std::string& es)
{
  auto vw = VW::initialize("--cb 2 --quiet");
  auto buffer = std::make_shared<std::vector<char>>();
  io_buf writer_view_of_buffer;
  writer_view_of_buffer.add_file(VW::io::create_vector_writer(buffer));
  vw->example_parser->output = &writer_view_of_buffer;
  vw->example_parser->write_cache = true;
  auto ae = &VW::get_unused_example(vw);

  VW::read_line(*vw, ae, const_cast<char*>(es.c_str()));

  if (vw->example_parser->write_cache)
  {
    vw->example_parser->lbl_parser.cache_label(&ae->l, *(vw->example_parser->output));
    cache_features(*(vw->example_parser->output), ae, vw->parse_mask);
  }
  vw->example_parser->output->flush();
  VW::finish_example(*vw, *ae);

  return buffer;
}

template <class... ExtraArgs>
static void bench_cache_io_buf(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::string res[sizeof...(extra_args)] = {extra_args...};
  auto example_string = res[0];

  auto buffer = get_cache_buffer(example_string);
  auto vw = VW::initialize("--cb 2 --quiet");

  auto examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));

  io_buf reader_view_of_buffer;
  vw->example_parser->input = &reader_view_of_buffer;

  for (auto _ : state)
  {
    for (size_t i = 0; i < 20000; i++)
    {
      reader_view_of_buffer.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size()));
      read_cached_features(vw, examples);
      VW::empty_example(*vw, *examples[0]);
      benchmark::ClobberMemory();
    }
  }
  examples.delete_v();
}

template <class... ExtraArgs>
static void bench_cache_io_buf_collections(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::string res[sizeof...(extra_args)] = {extra_args...};
  auto example_string = res[0];
  auto examples_size = std::stoi(res[1]);

  auto buffer = get_cache_buffer(example_string);
  auto vw = VW::initialize("--cb 2 --quiet");
  io_buf reader_view_of_buffer;
  vw->example_parser->input = &reader_view_of_buffer;

  auto examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));

  for (auto _ : state)
  {
    for (size_t i = 0; i < examples_size; i++)
    { reader_view_of_buffer.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size())); }
    while (read_cached_features(vw, examples)) { VW::empty_example(*vw, *examples[0]); }
    benchmark::ClobberMemory();
  }
  examples.delete_v();
}

template <class... ExtraArgs>
static void bench_text_io_buf(benchmark::State& state, ExtraArgs&&... extra_args)
{
  std::string res[sizeof...(extra_args)] = {extra_args...};
  auto example_string = res[0];

  auto vw = VW::initialize("--cb 2 --quiet");
  auto examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));

  io_buf reader_view_of_buffer;
  vw->example_parser->input = &reader_view_of_buffer;

  for (auto _ : state)
  {
    for (size_t i = 0; i < 20000; i++)
    {
      reader_view_of_buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
      vw->example_parser->reader(vw, examples);
      VW::empty_example(*vw, *examples[0]);
      benchmark::ClobberMemory();
    }
  }
  examples.delete_v();
}

BENCHMARK_CAPTURE(bench_text, 120_string_fts, get_x_string_fts(120));
BENCHMARK_CAPTURE(bench_cache_io_buf, 120_string_fts, get_x_string_fts(120));
BENCHMARK_CAPTURE(bench_text_io_buf, 120_string_fts, get_x_string_fts(120));

BENCHMARK_CAPTURE(bench_text, 120_num_fts, get_x_numerical_fts(120));
BENCHMARK_CAPTURE(bench_cache_io_buf, 120_num_fts, get_x_numerical_fts(120));
BENCHMARK_CAPTURE(bench_text_io_buf, 120_num_fts, get_x_numerical_fts(120));

// Run the benchmark
BENCHMARK_MAIN();