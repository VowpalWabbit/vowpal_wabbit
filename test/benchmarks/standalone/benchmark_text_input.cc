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
  VW::v_array<example*> examples;
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
  examples.push_back(VW::read_example(*vw, std::string("shared tag1| s_1 s_2")));
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

#ifdef PRIVACY_ACTIVATION
static void benchmark_cb_adf_learn_privacy_preserving(benchmark::State& state, int feature_count)
{
  auto vw = VW::initialize(
      "--privacy_activation --cb_explore_adf --epsilon 0.1 --quiet -q ::", nullptr, false, nullptr, nullptr);
  multi_ex examples;
  examples.push_back(VW::read_example(*vw, std::string("shared tag1| s_1 s_2")));
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
#endif

static void benchmark_ccb_adf_learn(benchmark::State& state, std::string feature_string, std::string cmd = "")
{
  auto vw = VW::initialize("--ccb_explore_adf --quiet" + cmd, nullptr, false, nullptr, nullptr);

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

static std::vector<std::vector<std::string>> gen_cb_examples(size_t num_examples,  // Total number of multi_ex examples
    size_t shared_feats_size,                                                      // Number of possible shared features
    size_t shared_feats_count,    // Number of shared features per multi_ex
    size_t actions_per_example,   // Number of actions in each multi_ex
    size_t feature_groups_size,   // Number of possible feature groups
    size_t feature_groups_count,  // Number of features groups per action
    size_t action_feats_size,     // Number of possible per-action features
    size_t action_feats_count,    // Number of actions per feature group per action
    bool same_first_char          // Flag to keep first character of all feature groups the same
)
{
  srand(0);
  std::vector<std::vector<std::string>> examples_vec;
  for (int ex = 0; ex < num_examples; ++ex)
  {
    std::vector<std::string> examples;
    std::ostringstream shared_ss;
    shared_ss << "shared |";
    for (int shared_feat = 0; shared_feat < shared_feats_count; ++shared_feat)
    { shared_ss << " " << (rand() % shared_feats_size); }
    examples.push_back(shared_ss.str());
    int action_ind = rand() % actions_per_example;
    for (int ac = 0; ac < actions_per_example; ++ac)
    {
      std::ostringstream action_ss;
      if (ac == action_ind) { action_ss << action_ind << ":1.0:0.5 "; }
      for (int fg = 0; fg < feature_groups_count; ++fg)
      {
        action_ss << "|";
        if (same_first_char) { action_ss << "f"; }
        action_ss << (static_cast<char>(65 + rand() % feature_groups_size)) << " ";
        for (int action_feat = 0; action_feat < action_feats_count; ++action_feat)
        { action_ss << (rand() % action_feats_size) << " "; }
      }
      examples.push_back(action_ss.str());
    }
    examples_vec.push_back(examples);
  }
  return examples_vec;
}

static std::vector<std::vector<std::string>> gen_ccb_examples(size_t num_examples,  // Total number of multi_ex examples
    size_t shared_feats_size,     // Number of possible shared features
    size_t shared_feats_count,    // Number of shared features per multi_ex
    size_t actions_per_example,   // Number of actions in each multi_ex
    size_t feature_groups_size,   // Number of possible feature groups
    size_t feature_groups_count,  // Number of features groups per action or slot
    size_t action_feats_size,     // Number of possible per-action/slot features
    size_t action_feats_count,    // Number of actions per feature group per action or slot
    bool same_first_char,         // Flag to keep first character of all feature groups the same
    size_t slots_per_example      // Number of slots
)
{
  srand(0);
  std::vector<std::vector<std::string>> examples_vec;
  for (int ex = 0; ex < num_examples; ++ex)
  {
    std::vector<std::string> examples;
    std::ostringstream shared_ss;
    shared_ss << "ccb shared |";
    for (int shared_feat = 0; shared_feat < shared_feats_count; ++shared_feat)
    { shared_ss << " " << (rand() % shared_feats_size); }
    examples.push_back(shared_ss.str());
    for (int ac = 0; ac < actions_per_example; ++ac)
    {
      std::ostringstream action_ss;
      action_ss << "ccb action ";
      for (int fg = 0; fg < feature_groups_count; ++fg)
      {
        action_ss << "|";
        if (same_first_char) { action_ss << "f"; }
        action_ss << ((char)(65 + rand() % feature_groups_size)) << " ";
        for (int action_feat = 0; action_feat < action_feats_count; ++action_feat)
        { action_ss << (rand() % action_feats_size) << " "; }
      }
      examples.push_back(action_ss.str());
    }
    for (int slot = 0; slot < slots_per_example; ++slot)
    {
      std::ostringstream slot_ss;
      slot_ss << "ccb slot ";
      for (int fg = 0; fg < feature_groups_count; ++fg)
      {
        slot_ss << (rand() % actions_per_example) << ":0." << (rand() % 10) << ":0." << (rand() % 10) << " |";
        if (same_first_char) { slot_ss << "f"; }
        slot_ss << ((char)(65 + rand() % feature_groups_size)) << " ";
        for (int slot_feat = 0; slot_feat < action_feats_count; ++slot_feat)
        { slot_ss << (rand() % action_feats_size) << " "; }
      }
      examples.push_back(slot_ss.str());
    }
    examples_vec.push_back(examples);
  }
  return examples_vec;
}

static std::vector<multi_ex> load_examples(VW::workspace* vw, const std::vector<std::vector<std::string>>& ex_strs)
{
  std::vector<multi_ex> examples_vec;
  for (const auto& ex_str : ex_strs)
  {
    multi_ex mxs;
    for (const auto& example : ex_str) { mxs.push_back(VW::read_example(*vw, example)); }
    examples_vec.push_back(mxs);
  }
  return examples_vec;
}

static void benchmark_multi(
    benchmark::State& state, const std::vector<std::vector<std::string>>& examples_str, const std::string& cmd)
{
  auto vw = VW::initialize(cmd, nullptr, false, nullptr, nullptr);
  std::vector<multi_ex> examples_vec = load_examples(vw, examples_str);
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
BENCHMARK_CAPTURE(benchmark_ccb_adf_learn, few_features_no_predict, "a", " --no_predict");
BENCHMARK_CAPTURE(benchmark_ccb_adf_learn, many_features_no_predic,
    "a b c d e f g h i j k l m n o p q r s t u v w x y z", " --no_predict");

BENCHMARK_CAPTURE(benchmark_cb_adf_learn, few_features, 2);
BENCHMARK_CAPTURE(benchmark_cb_adf_learn, many_features, 120)->MinTime(15.0);

#ifdef PRIVACY_ACTIVATION
BENCHMARK_CAPTURE(benchmark_cb_adf_learn_privacy_preserving, few_features, 2);
BENCHMARK_CAPTURE(benchmark_cb_adf_learn_privacy_preserving, many_features, 120);
#endif

BENCHMARK_CAPTURE(benchmark_multi, cb_adf_no_namespaces, gen_cb_examples(100, 7, 3, 6, 1, 4, 14, 2, false),
    "--cb_explore_adf --quiet")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, cb_adf_diff_char_no_interactions, gen_cb_examples(100, 7, 3, 6, 3, 4, 14, 2, false),
    "--cb_explore_adf --quiet")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, cb_adf_diff_char_interactions, gen_cb_examples(100, 7, 3, 6, 3, 4, 14, 2, false),
    "--cb_explore_adf --quiet -q ::")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, cb_adf_same_char_no_interactions, gen_cb_examples(100, 7, 3, 6, 3, 4, 14, 2, true),
    "--cb_explore_adf --quiet")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, cb_adf_same_char_interactions, gen_cb_examples(100, 7, 3, 6, 3, 4, 14, 2, true),
    "--cb_explore_adf --quiet -q ::")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, ccb_adf_no_namespaces, gen_ccb_examples(50, 7, 3, 6, 1, 4, 14, 2, false, 3),
    "--ccb_explore_adf --quiet")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, ccb_adf_diff_char_no_interactions,
    gen_ccb_examples(50, 7, 3, 6, 3, 4, 14, 2, false, 3), "--ccb_explore_adf --quiet")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, ccb_adf_diff_char_interactions, gen_ccb_examples(50, 7, 3, 6, 3, 4, 14, 2, false, 3),
    "--ccb_explore_adf --quiet -q ::")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, ccb_adf_same_char_no_interactions,
    gen_ccb_examples(50, 7, 3, 6, 3, 4, 14, 2, true, 3), "--ccb_explore_adf --quiet")->MinTime(15.0);
BENCHMARK_CAPTURE(benchmark_multi, ccb_adf_same_char_interactions, gen_ccb_examples(50, 7, 3, 6, 3, 4, 14, 2, true, 3),
    "--ccb_explore_adf --quiet -q ::")->MinTime(15.0);
