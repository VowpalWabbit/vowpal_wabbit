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
  v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(vw));
  for (auto _ : state)
  {
    VW::read_line(*vw, examples[0], es);
    VW::empty_example(*vw, *examples[0]);
    benchmark::ClobberMemory();
  }
  VW::finish(*vw);
}

static void benchmark_learn_simple(benchmark::State& state, std::string example_string)
{
  auto vw = VW::initialize("--quiet", nullptr, false, nullptr, nullptr);

  auto* example = VW::read_example(*vw, example_string);

  for (auto _ : state)
  {
    vw->learn(*example);
    benchmark::ClobberMemory();
  }
  vw->finish_example(*example);
  VW::finish(*vw);
}

static void benchmark_cb_adf_learn(benchmark::State& state, int feature_count)
{
  auto vw = VW::initialize("--cb_explore_adf --epsilon 0.1 --quiet -q ::", nullptr, false, nullptr, nullptr);
  multi_ex examples;
  examples.push_back(VW::read_example(*vw, std::string("shared | s_1 s_2")));
  examples.push_back(VW::read_example(*vw, get_x_string_fts(feature_count)));
  examples.push_back(VW::read_example(*vw, get_x_string_fts_no_label(feature_count)));
  examples.push_back(VW::read_example(*vw, get_x_string_fts_no_label(feature_count)));

  for (auto _ : state)
  {
    vw->learn(examples);
    benchmark::ClobberMemory();
  }
  vw->finish_example(examples);
  VW::finish(*vw);
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

  for (auto _ : state)
  {
    vw->learn(examples);
    benchmark::ClobberMemory();
  }
  vw->finish_example(examples);
  VW::finish(*vw);
}

static void benchmark_cb_adf_large(
    benchmark::State& state, int num_feature_groups, bool same_first_char, bool interactions)
{
  std::string cmd = "--cb_explore_adf --quiet";
  if (interactions) { cmd += " -q ::"; }
  auto vw = VW::initialize(cmd, nullptr, false, nullptr, nullptr);
  int example_size = 100;
  int actions_per_event = 6;
  int shared_feats_size = 7;
  int shared_feats_count = 3;
  int action_feats_size = 14;
  int action_feats_count = 4;
  std::vector<multi_ex> examples_vec;
  srand(0);
  for (int ex = 0; ex < example_size; ++ex)
  {
    multi_ex examples;
    std::ostringstream shared_ss;
    shared_ss << "shared |";
    for (int shared_feat = 0; shared_feat < shared_feats_count; ++shared_feat)
    { shared_ss << " " << (rand() % shared_feats_size); }
    examples.push_back(VW::read_example(*vw, shared_ss.str()));
    int action_ind = rand() % actions_per_event;
    for (int ac = 0; ac < actions_per_event; ++ac)
    {
      std::ostringstream action_ss;
      if (ac == action_ind) { action_ss << action_ind << ":1.0:0.5 "; }
      for (int action_feat = 0; action_feat < action_feats_count; ++action_feat)
      {
        action_ss << "|";
        if (same_first_char) { action_ss << "f"; }
        action_ss << (rand() % num_feature_groups);
        action_ss << " " << (rand() % action_feats_size) << " ";
      }
      examples.push_back(VW::read_example(*vw, action_ss.str()));
    }
    examples_vec.push_back(examples);
  }

  for (auto _ : state)
  {
    for (multi_ex examples : examples_vec) { vw->learn(examples); }
    benchmark::ClobberMemory();
  }
  for (multi_ex examples : examples_vec) { vw->finish_example(examples); }
  VW::finish(*vw);
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

BENCHMARK_CAPTURE(benchmark_cb_adf_large, no_namespaces, 1, false, false);
BENCHMARK_CAPTURE(benchmark_cb_adf_large, diff_char_no_interactions, 3, false, false);
BENCHMARK_CAPTURE(benchmark_cb_adf_large, diff_char_interactions, 3, false, true);
BENCHMARK_CAPTURE(benchmark_cb_adf_large, same_char_no_interactions, 3, true, false);
BENCHMARK_CAPTURE(benchmark_cb_adf_large, same_char_interactions, 3, true, true);
