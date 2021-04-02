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
    VW::read_line(*vw, examples[0], es);
    VW::empty_example(*vw, *examples[0]);
    benchmark::ClobberMemory();
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
    vw->example_parser->lbl_parser.cache_label(&ae->l, ae->_reduction_features, *(vw->example_parser->output));
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
    reader_view_of_buffer.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size()));
    read_cached_features(vw, examples);
    VW::empty_example(*vw, *examples[0]);
    benchmark::ClobberMemory();
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
    reader_view_of_buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
    vw->example_parser->reader(vw, examples);
    VW::empty_example(*vw, *examples[0]);
    benchmark::ClobberMemory();
  }
  examples.delete_v();
}

static void benchmark_example_reuse(benchmark::State& state)
{
  std::string example_string =
      "1 1.0 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures "
      "NumberOfLegs:4.0 HasStripes";

  auto vw = VW::initialize("--quiet", nullptr, false, nullptr, nullptr);

  auto examples = v_init<example*>();

  io_buf reader_view_of_buffer;
  vw->example_parser->input = &reader_view_of_buffer;

  for (auto _ : state)
  {
    examples.push_back(&VW::get_unused_example(vw));
    reader_view_of_buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
    vw->example_parser->reader(vw, examples);
    VW::finish_example(*vw, *examples[0]);
    examples.clear();
    benchmark::ClobberMemory();
  }
  examples.delete_v();
}

static void benchmark_learn_simple(benchmark::State& state, std::string example_string)
{
  auto vw = VW::initialize("--quiet", nullptr, false, nullptr, nullptr);

  auto* example = VW::read_example(*vw, example_string);
  VW::setup_example(*vw, example);

  for (auto _ : state)
  {
    vw->learn(*example);
    benchmark::ClobberMemory();
  }
  vw->finish_example(*example);
}

static void benchmark_cb_adf_learn(benchmark::State& state)
{
  auto vw = VW::initialize("--cb_explore_adf --epsilon 0.1 --quiet", nullptr, false, nullptr, nullptr);
  multi_ex examples;
  examples.push_back(VW::read_example(*vw, std::string("shared | s_1 s_2")));
  examples.push_back(VW::read_example(*vw, std::string("0:1.0:0.5 | a_1 b_1 c_1")));
  examples.push_back(VW::read_example(*vw, std::string("| a_2 b_2 c_2")));
  examples.push_back(VW::read_example(*vw, std::string("| a_3 b_3 c_3")));
  for (auto* example : examples) { VW::setup_example(*vw, example); }

  for (auto _ : state)
  {
    vw->learn(examples);
    benchmark::ClobberMemory();
  }
  vw->finish_example(examples);
}

static void benchmark_ccb_adf_learn(benchmark::State& state, std::string feature_string)
{
  auto vw = VW::initialize("--ccb_explore_adf --quiet", nullptr, false, nullptr, nullptr);

  multi_ex examples;
  examples.push_back(VW::read_example(*vw, std::string("ccb shared |User " + feature_string)));
  examples.push_back(VW::read_example(*vw, std::string("ccb action |Action1 " + feature_string)));
  examples.push_back(VW::read_example(*vw, std::string("ccb action |Action2 " + feature_string)));
  examples.push_back(VW::read_example(*vw, std::string("ccb action |Action3 " + feature_string)));
  examples.push_back(VW::read_example(*vw, std::string("ccb action |Action4 " + feature_string)));
  examples.push_back(VW::read_example(*vw, std::string("ccb action |Action5 " + feature_string)));
  examples.push_back(VW::read_example(*vw, std::string("ccb slot 0:0:0.2 |Slot h")));
  examples.push_back(VW::read_example(*vw, std::string("ccb slot 1:0:0.25 |Slot i")));
  examples.push_back(VW::read_example(*vw, std::string("ccb slot 2:0:0.333333 |Slot j")));
  for (auto* example : examples) { VW::setup_example(*vw, example); }

  for (auto _ : state)
  {
    vw->learn(examples);
    benchmark::ClobberMemory();
  }
  vw->finish_example(examples);
}

BENCHMARK_CAPTURE(bench_text, 120_string_fts, get_x_string_fts(120));
BENCHMARK_CAPTURE(bench_cache_io_buf, 120_string_fts, get_x_string_fts(120));
BENCHMARK_CAPTURE(bench_text_io_buf, 120_string_fts, get_x_string_fts(120));

BENCHMARK_CAPTURE(bench_text, 120_num_fts, get_x_numerical_fts(120));
BENCHMARK_CAPTURE(bench_cache_io_buf, 120_num_fts, get_x_numerical_fts(120));
BENCHMARK_CAPTURE(bench_text_io_buf, 120_num_fts, get_x_numerical_fts(120));

BENCHMARK_CAPTURE(benchmark_learn_simple, 8_features,
    "1 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures NumberOfLegs:4.0 "
    "HasStripes");
BENCHMARK_CAPTURE(benchmark_learn_simple, 1_feature, "1 | a");

BENCHMARK_CAPTURE(benchmark_ccb_adf_learn, few_features, "a");
BENCHMARK_CAPTURE(benchmark_ccb_adf_learn, many_features, "a b c d e f g h i j k l m n o p q r s t u v w x y z");

BENCHMARK(benchmark_example_reuse);
BENCHMARK(benchmark_cb_adf_learn);
