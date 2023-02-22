// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "qr_decomposition.h"
#include "reductions/cb/details/large_action/compute_dot_prod_scalar.h"
#include "reductions/cb/details/large_action/compute_dot_prod_simd.h"
#include "reductions/cb/details/large_action_space.h"
#include "vw/common/random.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

using internal_action_space_op =
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space<
        VW::cb_explore_adf::one_pass_svd_impl, VW::cb_explore_adf::one_rank_spanner_state>>;

TEST(Las, CheckMatricsWithLASRunsOK)
{
  auto d = 3;
  std::vector<std::string> args{"--cb_explore_adf", "--large_action_space", "--max_actions", std::to_string(d),
      "--quiet", "--extra_metrics", "las_metrics.json"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  VW::multi_ex examples;

  examples.push_back(VW::read_example(*vw, "1:1:0.1 | 1:0.1 2:0.12 3:0.13"));
  examples.push_back(VW::read_example(*vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
  examples.push_back(VW::read_example(*vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
  examples.push_back(VW::read_example(*vw, "| a_7 a_8 a_9"));
  examples.push_back(VW::read_example(*vw, "| a_10 a_11 a_12"));
  examples.push_back(VW::read_example(*vw, "| a_13 a_14 a_15"));
  examples.push_back(VW::read_example(*vw, "| a_16 a_17 a_18"));

  vw->learn(examples);

  auto num_actions = examples[0]->pred.a_s.size();

  EXPECT_EQ(num_actions, 7);

  vw->finish_example(examples);

  auto metrics = vw->global_metrics.collect_metrics(vw->l.get());
  EXPECT_EQ(metrics.get_uint("cbea_labeled_ex"), 1);
  EXPECT_EQ(metrics.get_uint("cb_las_filtering_factor"), 5);
}

TEST(Las, CheckAOSameActionsSameRepresentation)
{
  auto d = 3;
  std::vector<std::unique_ptr<VW::workspace>> vws;
  for (const int seed : {1, 0})
  {
    for (const bool use_simd : {false, true})
    {
      std::vector<std::string> args{"--cb_explore_adf", "--large_action_space", "--max_actions", std::to_string(d),
          "--quiet", "--random_seed", std::to_string(seed)};
      if (use_simd) { args.emplace_back("--las_hint_explicit_simd"); }
      vws.push_back(VW::initialize(VW::make_unique<VW::config::options_cli>(args)));
    }
  }

  for (auto& vw_ptr : vws)
  {
    auto& vw = *vw_ptr;

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto* action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

    action_space->explore._populate_all_testing_components();

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "shared |U b c"));
      examples.push_back(VW::read_example(vw, "|A 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));
      // duplicates start
      examples.push_back(VW::read_example(vw, "|A a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      examples.push_back(VW::read_example(vw, "|A a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      // duplicates end
      examples.push_back(VW::read_example(vw, "|A a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10:0.2"));
      examples.push_back(VW::read_example(vw, "|A a_7 a_8 a_9 v1:0.99"));
      examples.push_back(VW::read_example(vw, "|A a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "|A a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "|A a_16 a_17 a_18:0.2"));

      vw.predict(examples);

      // representation of actions 2 and 3 (duplicates) should be the same in U
      EXPECT_TRUE(action_space->explore.U.row(1).isApprox(action_space->explore.U.row(2), vwtest::EXPLICIT_FLOAT_TOL));

      vw.finish_example(examples);
    }
  }
}

TEST(Las, CheckAOLinearCombinationOfActions)
{
  auto d = 3;
  std::vector<std::unique_ptr<VW::workspace>> vws;
  for (const int seed : {3, 0})
  {
    for (const bool use_simd : {false, true})
    {
      std::vector<std::string> args{"--cb_explore_adf", "--large_action_space", "--max_actions", std::to_string(d),
          "--quiet", "--noconstant", "--random_seed", std::to_string(seed)};
      if (use_simd) { args.emplace_back("--las_hint_explicit_simd"); }
      vws.push_back(VW::initialize(VW::make_unique<VW::config::options_cli>(args)));
    }
  }

  for (auto& vw_ptr : vws)
  {
    auto& vw = *vw_ptr;

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto* action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

    action_space->explore._populate_all_testing_components();

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "shared |U b c"));
      examples.push_back(VW::read_example(vw, "|A 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));

      examples.push_back(VW::read_example(vw, "|A a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      examples.push_back(VW::read_example(vw, "|A a_1:0.8 a_2:0.32 a_3:0.15 a100:0.2 a200:0.2"));
      // linear combination of the above two actions
      // action_4 = action_2 + 2 * action_3
      examples.push_back(VW::read_example(vw, "|A a_1:2.1 a_2:1.29 a_3:0.42 a100:4.4 a200:33.4"));

      examples.push_back(VW::read_example(vw, "|A a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10: 0.2"));
      examples.push_back(VW::read_example(vw, "|A a_7 a_8 a_9 v1:0.99"));
      examples.push_back(VW::read_example(vw, "|A a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "|A a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "|A a_16 a_17 a_18:0.2"));

      vw.predict(examples);

      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "shared |U b c"));
      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));

      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      examples.push_back(VW::read_example(vw, "| a_1:0.8 a_2:0.32 a_3:0.15 a100:0.2 a200:0.2"));
      // linear combination of the above two actions
      // action_4 = action_2 + 2 * action_3
      examples.push_back(VW::read_example(vw, "| a_1:2.1 a_2:1.29 a_3:0.42 a100:4.4 a200:33.4"));

      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10: 0.2"));
      examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9 v1:0.99"));
      examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18:0.2"));

      vw.learn(examples);

      // check that the representation of the fourth action is the same linear combination of the representation of the
      // 2nd and 3rd actions
      Eigen::VectorXf action_2 = action_space->explore.U.row(1);
      Eigen::VectorXf action_3 = action_space->explore.U.row(2);
      Eigen::VectorXf action_4 = action_space->explore.U.row(3);

      Eigen::VectorXf action_lin_rep = action_2 + 2.f * action_3;

      EXPECT_TRUE(action_lin_rep.isApprox(action_4, vwtest::EXPLICIT_FLOAT_TOL));

      vw.finish_example(examples);
    }
  }
}

#ifdef BUILD_LAS_WITH_SIMD
TEST(Las, ComputeDotProdScalarAndSimdHaveSameResults)
{
  float (*compute_dot_prod_simd)(uint64_t, VW::workspace*, uint64_t, VW::example*);
  if (VW::cb_explore_adf::cpu_supports_avx512())
  {
    compute_dot_prod_simd = VW::cb_explore_adf::compute_dot_prod_avx512;
  }
  else if (VW::cb_explore_adf::cpu_supports_avx2())
  {
    compute_dot_prod_simd = VW::cb_explore_adf::compute_dot_prod_avx2;
  }
  else
  {
    // Skip this test because of no supported simd implementations.
    return;
  }

  auto generate_example = [](int num_namespaces, int num_features)
  {
    std::string s;
    for (int i = 0; i < num_namespaces; ++i)
    {
      s += " |";
      s += static_cast<char>('A' + i);
      for (int j = 0; j < num_features; ++j)
      {
        s += std::string(" ") + static_cast<char>('a' + i) + std::to_string(rand() % 1000);
      }
    }
    return s;
  };
  constexpr uint64_t column_index = 666;
  constexpr uint64_t seed = 233;

  {
    // No interactions, few features
    auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--quiet"));
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw, generate_example(/*num_namespaces=*/2, /*num_features=*/5)));
    auto* ex = examples[0];
    auto interactions =
        VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
            vw->interactions, std::set<VW::namespace_index>(ex->indices.begin(), ex->indices.end()));
    ex->interactions = &interactions;
    EXPECT_EQ(interactions.size(), 0);

    float result_scalar = VW::cb_explore_adf::compute_dot_prod_scalar(column_index, vw.get(), seed, ex);
    float result_simd = compute_dot_prod_simd(column_index, vw.get(), seed, ex);
    EXPECT_FLOAT_EQ(result_simd, result_scalar);
    vw->finish_example(examples);
  }
  {
    // No interactions, many features
    auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--quiet"));
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw, generate_example(/*num_namespaces=*/2, /*num_features=*/50)));
    auto* ex = examples[0];
    auto interactions =
        VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
            vw->interactions, std::set<VW::namespace_index>(ex->indices.begin(), ex->indices.end()));
    ex->interactions = &interactions;
    EXPECT_EQ(interactions.size(), 0);

    float result_scalar = VW::cb_explore_adf::compute_dot_prod_scalar(column_index, vw.get(), seed, ex);
    float result_simd = compute_dot_prod_simd(column_index, vw.get(), seed, ex);
    EXPECT_FLOAT_EQ(result_simd, result_scalar);
    vw->finish_example(examples);
  }
  {
    // Quadratics, few features
    auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--quiet", "-q::"));
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw, generate_example(/*num_namespaces=*/2, /*num_features=*/5)));
    auto* ex = examples[0];
    auto interactions =
        VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
            vw->interactions, std::set<VW::namespace_index>(ex->indices.begin(), ex->indices.end()));
    ex->interactions = &interactions;
    EXPECT_EQ(interactions.size(), 6);

    float result_scalar = VW::cb_explore_adf::compute_dot_prod_scalar(column_index, vw.get(), seed, ex);
    float result_simd = compute_dot_prod_simd(column_index, vw.get(), seed, ex);
    EXPECT_FLOAT_EQ(result_simd, result_scalar);
    vw->finish_example(examples);
  }
  {
    // Quadratics, many features
    auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--quiet", "-q::"));
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw, generate_example(/*num_namespaces=*/2, /*num_features=*/50)));
    auto* ex = examples[0];
    auto interactions =
        VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
            vw->interactions, std::set<VW::namespace_index>(ex->indices.begin(), ex->indices.end()));
    ex->interactions = &interactions;
    EXPECT_EQ(interactions.size(), 6);

    float result_scalar = VW::cb_explore_adf::compute_dot_prod_scalar(column_index, vw.get(), seed, ex);
    float result_simd = compute_dot_prod_simd(column_index, vw.get(), seed, ex);
    EXPECT_FLOAT_EQ(result_simd, result_scalar);
    vw->finish_example(examples);
  }
}

TEST(Las, ScalarAndSimdGenerateSamePredictions)
{
  const bool cpu_supports_simd = (VW::cb_explore_adf::cpu_supports_avx512() || VW::cb_explore_adf::cpu_supports_avx2());

  auto generate_example = [](int num_namespaces, int num_features)
  {
    std::string s;
    for (int i = 0; i < num_namespaces; ++i)
    {
      s += " |";
      s += static_cast<char>('A' + i);
      for (int j = 0; j < num_features; ++j)
      {
        s += std::string(" ") + static_cast<char>('a' + i) + std::to_string(rand() % 1000);
      }
    }
    return s;
  };
  const int num_actions = 30;
  std::vector<std::string> examples;
  for (int i = 0; i < num_actions; ++i)
  {
    examples.push_back(
        generate_example(/*num_namespaces=*/std::max(1, rand() % 5), /*num_features=*/std::max(1, rand() % 30)));
  }

  {
    // No interactions
    std::vector<std::string> vw_cmd{"--cb_explore_adf", "--large_action_space", "--quiet"};

    auto vw_scalar = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_cmd));

    VW::LEARNER::learner* learner_scalar =
        require_multiline(vw_scalar->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space_scalar =
        (internal_action_space_op*)learner_scalar->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space_scalar, nullptr);

    EXPECT_FALSE(action_space_scalar->explore.impl._test_only_use_simd());

    VW::multi_ex ex_scalar;
    for (const auto& example : examples) { ex_scalar.push_back(VW::read_example(*vw_scalar, example)); }
    vw_scalar->predict(ex_scalar);
    auto& scores_scalar = ex_scalar[0]->pred.a_s;

    vw_cmd.push_back("--las_hint_explicit_simd");
    auto vw_simd = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_cmd));

    VW::LEARNER::learner* learner_simd =
        require_multiline(vw_simd->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space_simd =
        (internal_action_space_op*)learner_simd->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space_simd, nullptr);

    if (cpu_supports_simd) { EXPECT_TRUE(action_space_simd->explore.impl._test_only_use_simd()); }
    else { EXPECT_FALSE(action_space_simd->explore.impl._test_only_use_simd()); }

    VW::multi_ex ex_simd;
    for (const auto& example : examples) { ex_simd.push_back(VW::read_example(*vw_simd, example)); }
    vw_simd->predict(ex_simd);
    auto& scores_simd = ex_simd[0]->pred.a_s;

    EXPECT_EQ(scores_scalar.size(), scores_simd.size());
    for (size_t i = 0; i < scores_scalar.size(); ++i)
    {
      EXPECT_EQ(scores_scalar[i].action, scores_simd[i].action);
      EXPECT_FLOAT_EQ(scores_scalar[i].score, scores_simd[i].score);
    }

    vw_scalar->finish_example(ex_scalar);
    vw_simd->finish_example(ex_simd);
  }
  {
    // Quadratic interactions
    std::vector<std::string> vw_cmd{"--cb_explore_adf", "--large_action_space", "--quiet", "-q::"};

    auto vw_scalar = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_cmd));

    VW::LEARNER::learner* learner_scalar =
        require_multiline(vw_scalar->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space_scalar =
        (internal_action_space_op*)learner_scalar->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space_scalar, nullptr);

    EXPECT_FALSE(action_space_scalar->explore.impl._test_only_use_simd());

    VW::multi_ex ex_scalar;
    for (const auto& example : examples) { ex_scalar.push_back(VW::read_example(*vw_scalar, example)); }
    vw_scalar->predict(ex_scalar);
    auto& scores_scalar = ex_scalar[0]->pred.a_s;

    vw_cmd.push_back("--las_hint_explicit_simd");
    auto vw_simd = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_cmd));

    VW::LEARNER::learner* learner_simd =
        require_multiline(vw_simd->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space_simd =
        (internal_action_space_op*)learner_simd->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space_simd, nullptr);

    if (cpu_supports_simd) { EXPECT_TRUE(action_space_simd->explore.impl._test_only_use_simd()); }
    else { EXPECT_FALSE(action_space_simd->explore.impl._test_only_use_simd()); }

    VW::multi_ex ex_simd;
    for (const auto& example : examples) { ex_simd.push_back(VW::read_example(*vw_simd, example)); }
    vw_simd->predict(ex_simd);
    auto& scores_simd = ex_simd[0]->pred.a_s;

    EXPECT_EQ(scores_scalar.size(), scores_simd.size());
    for (size_t i = 0; i < scores_scalar.size(); ++i)
    {
      EXPECT_EQ(scores_scalar[i].action, scores_simd[i].action);
      EXPECT_FLOAT_EQ(scores_scalar[i].score, scores_simd[i].score);
    }

    vw_scalar->finish_example(ex_scalar);
    vw_simd->finish_example(ex_simd);
  }
  {
    // Ignore & ignore_linear
    std::vector<std::string> vw_cmd{
        "--cb_explore_adf", "--large_action_space", "--quiet", "-q::", "--ignore=A", "--ignore_linear=B"};

    auto vw_scalar = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_cmd));

    VW::LEARNER::learner* learner_scalar =
        require_multiline(vw_scalar->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space_scalar =
        (internal_action_space_op*)learner_scalar->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space_scalar, nullptr);

    EXPECT_FALSE(action_space_scalar->explore.impl._test_only_use_simd());

    VW::multi_ex ex_scalar;
    for (const auto& example : examples) { ex_scalar.push_back(VW::read_example(*vw_scalar, example)); }
    vw_scalar->predict(ex_scalar);
    auto& scores_scalar = ex_scalar[0]->pred.a_s;

    vw_cmd.push_back("--las_hint_explicit_simd");
    auto vw_simd = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_cmd));

    VW::LEARNER::learner* learner_simd =
        require_multiline(vw_simd->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space_simd =
        (internal_action_space_op*)learner_simd->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space_simd, nullptr);

    if (cpu_supports_simd) { EXPECT_TRUE(action_space_simd->explore.impl._test_only_use_simd()); }
    else { EXPECT_FALSE(action_space_simd->explore.impl._test_only_use_simd()); }

    VW::multi_ex ex_simd;
    for (const auto& example : examples) { ex_simd.push_back(VW::read_example(*vw_simd, example)); }
    vw_simd->predict(ex_simd);
    auto& scores_simd = ex_simd[0]->pred.a_s;

    EXPECT_EQ(scores_scalar.size(), scores_simd.size());
    for (size_t i = 0; i < scores_scalar.size(); ++i)
    {
      EXPECT_EQ(scores_scalar[i].action, scores_simd[i].action);
      EXPECT_FLOAT_EQ(scores_scalar[i].score, scores_simd[i].score);
    }

    vw_scalar->finish_example(ex_scalar);
    vw_simd->finish_example(ex_simd);
  }
  {
    // Cubics & generic interactions are not supported yet
    auto vw_simd = VW::initialize(vwtest::make_args(
        "--cb_explore_adf", "--large_action_space", "--quiet", "--cubic", ":::", "--las_hint_explicit_simd"));

    VW::LEARNER::learner* learner =
        require_multiline(vw_simd->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space, nullptr);

    EXPECT_FALSE(action_space->explore.impl._test_only_use_simd());
  }
  {
    // Extent interactions are not supported yet
    auto vw_simd = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--quiet",
        "--experimental_full_name_interactions", "A|B", "--las_hint_explicit_simd"));

    VW::LEARNER::learner* learner =
        require_multiline(vw_simd->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));
    auto* action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
    EXPECT_NE(action_space, nullptr);

    EXPECT_FALSE(action_space->explore.impl._test_only_use_simd());
  }
}
#endif
