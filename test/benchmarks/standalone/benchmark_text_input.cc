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

#include "vw.h"
#include "../benchmarks_common.h"

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

static void benchmark_cb_adf_learn(benchmark::State& state, int feature_count)
{
  auto vw = VW::initialize("--cb_explore_adf --epsilon 0.1 --quiet -q ::", nullptr, false, nullptr, nullptr);
  multi_ex examples;
  examples.push_back(VW::read_example(*vw, std::string("shared | s_1 s_2")));
  examples.push_back(VW::read_example(*vw, get_x_string_fts(feature_count)));
  examples.push_back(VW::read_example(*vw, get_x_string_fts_no_label(feature_count)));
  examples.push_back(VW::read_example(*vw, get_x_string_fts_no_label(feature_count)));
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
BENCHMARK_CAPTURE(bench_text, 120_num_fts, get_x_numerical_fts(120));

BENCHMARK_CAPTURE(benchmark_learn_simple, 8_features,
    "1 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures NumberOfLegs:4.0 "
    "HasStripes");
BENCHMARK_CAPTURE(benchmark_learn_simple, 1_feature, "1 | a");

BENCHMARK_CAPTURE(benchmark_ccb_adf_learn, few_features, "a");
BENCHMARK_CAPTURE(benchmark_ccb_adf_learn, many_features, "a b c d e f g h i j k l m n o p q r s t u v w x y z");

BENCHMARK_CAPTURE(benchmark_cb_adf_learn, few_features, 2);
BENCHMARK_CAPTURE(benchmark_cb_adf_learn, many_features, 120);
