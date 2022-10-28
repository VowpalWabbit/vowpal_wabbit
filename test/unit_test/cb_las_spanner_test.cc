// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions/cb/details/large_action_space.h"
#include "test_common.h"
#include "vw/core/qr_decomposition.h"
#include "vw/core/rand48.h"
#include "vw/core/rand_state.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"
#include "vw/core/vw.h"

#include <boost/test/unit_test.hpp>

using internal_action_space_op =
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space<
        VW::cb_explore_adf::one_pass_svd_impl, VW::cb_explore_adf::one_rank_spanner_state>>;

BOOST_AUTO_TEST_SUITE(test_suite_las_spanner)

BOOST_AUTO_TEST_CASE(check_finding_max_volume)
{
  auto d = 3;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 0", nullptr,
      false, nullptr, nullptr);
  uint64_t seed = vw.get_random_state()->get_current_state() * 10.f;
  VW::cb_explore_adf::cb_explore_adf_large_action_space<VW::cb_explore_adf::one_pass_svd_impl,
      VW::cb_explore_adf::one_rank_spanner_state>
      largecb(
          /*d=*/0, /*gamma_scale=*/1.f, /*gamma_exponent=*/0.f, /*c=*/2, false, &vw, seed, 1 << vw.num_bits,
          /*thread_pool_size*/ 0, /*block_size*/ 0, VW::cb_explore_adf::implementation_type::one_pass_svd);
  largecb.U = Eigen::MatrixXf{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {0, 0, 0}, {7, 5, 3}, {6, 4, 8}};
  Eigen::MatrixXf X{{1, 2, 3}, {3, 2, 1}, {2, 1, 3}};

  float max_volume;
  uint64_t U_rid;

  // volume is scaled with one rank spanner

  Eigen::MatrixXf X_inv = X.inverse();
  Eigen::VectorXf phi = X_inv.row(0);
  largecb.spanner_state.find_max_volume(largecb.U, phi, max_volume, U_rid);
  BOOST_CHECK_SMALL(max_volume - 2.08333349f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(U_rid, 2);

  phi = X_inv.row(1);
  largecb.spanner_state.find_max_volume(largecb.U, phi, max_volume, U_rid);
  BOOST_CHECK_SMALL(max_volume - 3.33333278f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(U_rid, 4);

  phi = X_inv.row(2);
  largecb.spanner_state.find_max_volume(largecb.U, phi, max_volume, U_rid);
  BOOST_CHECK_SMALL(max_volume - 2.16666675f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(U_rid, 5);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(check_spanner_results_squarecb)
{
  auto d = 2;
  std::vector<VW::workspace*> vws;

  auto* vw = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " + std::to_string(d) +
          " --quiet --random_seed 1",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw);

  auto* vw_zs = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " + std::to_string(d) +
          " --quiet --random_seed 0",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_zs);

  for (auto* vw_ptr : vws)
  {
    auto& vw = *vw_ptr;

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
      examples.push_back(VW::read_example(vw, "| a_7:0.9 a_8:0.05 a_9:0.45"));
      examples.push_back(VW::read_example(vw, "| a_10:5.9 a_11:5.05 a_12:5.45"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
      examples.push_back(VW::read_example(vw, "| a_7:0.9 a_8:0.05 a_9:0.45"));
      examples.push_back(VW::read_example(vw, "| a_10:5.9 a_11:5.05 a_12:5.45"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_4:0.8 a_5:0.32 a_6:0.15"));
      examples.push_back(VW::read_example(vw, "| a_7:0.9 a_8:0.05 a_9:0.45"));
      examples.push_back(VW::read_example(vw, "| a_10:5.9 a_11:5.05 a_12:5.45"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_7:0.9 a_8:0.05 a_9:0.45"));
      examples.push_back(VW::read_example(vw, "| a_10:5.9 a_11:5.05 a_12:5.45"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
      examples.push_back(VW::read_example(vw, "| a_7:0.9 a_8:0.05 a_9:0.45"));
      examples.push_back(VW::read_example(vw, "0:0.1:0.5 | a_10:5.9 a_11:5.05 a_12:5.45"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
    }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
      examples.push_back(VW::read_example(vw, "| a_7:0.9 a_8:0.05 a_9:0.45"));
      examples.push_back(VW::read_example(vw, "| a_10:5.9 a_11:5.05 a_12:5.45"));

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;

      // Only d actions have non-zero scores.
      BOOST_CHECK_EQUAL(preds.size(), num_actions);

      // max should be in place 0
      float max_prob = preds[0].score;
      for (size_t i = 1; i < preds.size(); i++) { BOOST_CHECK_LT(preds[i].score, max_prob); }

      size_t count_zero_scores = 0;
      for (const auto& as : preds)
      {
        if (as.score == 0.f) { count_zero_scores++; }
      }

      BOOST_CHECK_LE(count_zero_scores, num_actions - d);
      BOOST_CHECK_GE(count_zero_scores, num_actions - (d + 1));

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_spanner_results_epsilon_greedy)
{
  auto d = 2;
  float epsilon = 0.2f;

  std::vector<VW::workspace*> vws;

  auto* vw = VW::initialize("--cb_explore_adf --epsilon " + std::to_string(epsilon) +
          " --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 3 --thread_pool_size 4",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw);

  auto* vw_zs = VW::initialize("--cb_explore_adf --epsilon " + std::to_string(epsilon) +
          " --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 0",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_zs);

  for (auto* vw_ptr : vws)
  {
    auto& vw = *vw_ptr;

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
    }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;
      // Only d actions have non-zero scores.
      BOOST_CHECK_EQUAL(preds.size(), num_actions);

      size_t num_actions_non_zeroed = d;
      float epsilon_ur = epsilon / num_actions_non_zeroed;
      BOOST_CHECK_SMALL(preds[0].score - (epsilon_ur + (1.f - epsilon)), FLOAT_TOL);
      BOOST_CHECK_EQUAL(preds[0].action, 0);

      // check either 1 or 2 but not both since they are a duplicate

      size_t encounters = 0;
      for (auto& a_s : preds)
      {
        if (a_s.action == 1 && a_s.score != 0.f)
        {
          encounters++;
          BOOST_CHECK_SMALL(a_s.score - epsilon_ur, FLOAT_TOL);
        }
        if (a_s.action == 2 && a_s.score != 0.f)
        {
          encounters++;
          BOOST_CHECK_SMALL(a_s.score - epsilon_ur, FLOAT_TOL);
        }
      }

      BOOST_CHECK_EQUAL(encounters, 1);

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_uniform_probabilities_before_learning)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_epsilon = VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) +
          " --quiet --noconstant --two_pass_svd",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_epsilon, false});

  auto* vw_squarecb = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --noconstant --two_pass_svd",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_squarecb, true});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto apply_diag_M = std::get<1>(vw_pair);

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1"));
      examples.push_back(VW::read_example(vw, "| 1"));
      examples.push_back(VW::read_example(vw, "| 1"));

      learner->predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;
      BOOST_CHECK_EQUAL(preds.size(), num_actions);
      for (const auto& pred : preds) { BOOST_CHECK_SMALL(pred.score - (1.f / 3.f), FLOAT_TOL); }

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_probabilities_when_d_is_larger)
{
  auto d = 3;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.learn(examples);
    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.learn(examples);
    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.learn(examples);
    vw.finish_example(examples);
  }

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  {
    BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
  }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.predict(examples);

    const auto num_actions = examples.size();
    const auto& preds = examples[0]->pred.a_s;
    BOOST_CHECK_EQUAL(preds.size(), num_actions);
    BOOST_CHECK_SMALL(preds[0].score - 0.966666639f, FLOAT_TOL);
    BOOST_CHECK_SMALL(preds[1].score - 0.0166666675f, FLOAT_TOL);
    BOOST_CHECK_SMALL(preds[2].score - 0.0166666675f, FLOAT_TOL);

    vw.finish_example(examples);
  }
  VW::finish(vw);
}

static std::vector<std::string> gen_cb_examples(
    size_t actions_per_example, size_t coordinates, float scale, bool add_cost = true)
{
  srand(0);
  std::vector<std::string> examples;

  int action_ind = rand() % actions_per_example;
  for (int ac = 0; ac < actions_per_example; ++ac)
  {
    std::ostringstream action_ss;
    if (ac == action_ind && add_cost) { action_ss << action_ind << ":1.0:0.5 "; }

    action_ss << "| ";
    for (int action_feat = 0; action_feat < coordinates; ++action_feat)
    {
      action_ss << "x" << action_feat << ":" << ((static_cast<double>(std::rand()) / RAND_MAX) * scale) << " ";
    }

    examples.push_back(action_ss.str());
  }

  return examples;
}

BOOST_AUTO_TEST_CASE(check_spanner_chooses_actions_that_clearly_maximise_volume)
{
  // d actions with larger values (factor of 10x)
  // 10d - d (the rest) actions with smaller values
  // expect the d actions to be chosen by the spanner

  auto d = 5;
  auto K = 10 * d;

  auto exs = gen_cb_examples(K - d, 10, 1.f);
  auto dexs = gen_cb_examples(d, 10, 100.f, false);

  std::vector<VW::workspace*> vws;

  auto* vw_squarecb = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --random_seed 6",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_squarecb);

  auto* vw_squarecb_zs = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --random_seed 0",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_squarecb_zs);

  for (auto* vw_ptr : vws)
  {
    auto& vw = *vw_ptr;

    {
      VW::multi_ex examples;

      for (auto ex : dexs) { examples.push_back(VW::read_example(vw, ex)); }
      for (auto ex : exs) { examples.push_back(VW::read_example(vw, ex)); }

      vw.learn(examples);
      vw.finish_example(examples);
    }

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
    }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
    BOOST_CHECK_EQUAL(action_space != nullptr, true);
    action_space->explore._populate_all_testing_components();

    {
      VW::multi_ex examples;

      for (auto ex : exs) { examples.push_back(VW::read_example(vw, ex)); }
      // check that the LAST 5 examples are chosen in the spanner
      for (auto ex : dexs) { examples.push_back(VW::read_example(vw, ex)); }

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;

      size_t count_non_zero_scores = 0;
      size_t count_zero_scores = 0;
      size_t last_5_actions_non_zero = 0;

      for (auto a_s : preds)
      {
        if (a_s.action < K - d)
        {
          if (a_s.score == 0.f) { count_zero_scores++; }
          else { count_non_zero_scores++; }
        }
        else
        {
          if (a_s.score != 0.f)
          {
            count_non_zero_scores++;
            last_5_actions_non_zero++;
          }
          else { count_zero_scores++; }
        }
      }

      BOOST_CHECK_EQUAL(last_5_actions_non_zero, d);

      // either VW's prediction was in the spanner and so we have exactly d non-zero scores
      // or it was not in the spanner so got forcefully added before returning the predictions so we have d + 1 non-zero
      // scores

      BOOST_CHECK_LE(count_non_zero_scores, d + 1);
      BOOST_CHECK_GE(count_non_zero_scores, d);

      BOOST_CHECK_LE(count_zero_scores, preds.size() - d);
      BOOST_CHECK_GE(count_zero_scores, preds.size() - (d + 1));

      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      // check that the FIRST 5 examples are chosen in the spanner
      for (auto ex : dexs) { examples.push_back(VW::read_example(vw, ex)); }
      for (auto ex : exs) { examples.push_back(VW::read_example(vw, ex)); }

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;

      size_t count_non_zero_scores = 0;
      size_t count_zero_scores = 0;
      size_t first_5_actions_non_zero = 0;
      for (auto a_s : preds)
      {
        if (a_s.action < d)
        {
          if (a_s.score != 0.f)
          {
            count_non_zero_scores++;
            first_5_actions_non_zero++;
          }
          else { count_zero_scores++; }
        }
        else
        {
          if (a_s.score == 0.f) { count_zero_scores++; }
          else { count_non_zero_scores++; }
        }
      }

      BOOST_CHECK_EQUAL(first_5_actions_non_zero, d);

      // either VW's prediction was in the spanner and so we have exactly d non-zero scores
      // or it was not in the spanner so got forcefully added before returning the predictions so we have d + 1 non-zero
      // scores

      BOOST_CHECK_LE(count_non_zero_scores, d + 1);
      BOOST_CHECK_GE(count_non_zero_scores, d);

      BOOST_CHECK_LE(count_zero_scores, preds.size() - d);
      BOOST_CHECK_GE(count_zero_scores, preds.size() - (d + 1));

      vw.finish_example(examples);
    }

    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_spanner_rejects_same_actions)
{
  // 8 actions and I want spanner to reject the duplicate
  auto d = 7;
  std::vector<VW::workspace*> vws;

  auto* vw_squarecb = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --random_seed 8",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_squarecb);

  auto* vw_squarecb_zs = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --random_seed 0",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_squarecb_zs);

  for (auto* vw_ptr : vws)
  {
    auto& vw = *vw_ptr;

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));
      // duplicates start
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      // duplicates end
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10:0.2"));
      examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9 v1:0.99"));
      examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18:0.2"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
    }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
    BOOST_CHECK_EQUAL(action_space != nullptr, true);
    action_space->explore._populate_all_testing_components();

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));
      // duplicates start
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
      // duplicates end
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10:0.2"));
      examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9 v1:0.99"));
      examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18:0.2"));

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;

      size_t encounters = 0;
      for (auto& a_s : preds)
      {
        if (a_s.action == 1 && a_s.score != 0.f) { encounters++; }
        if (a_s.action == 2 && a_s.score != 0.f) { encounters++; }
      }

      BOOST_CHECK_EQUAL(encounters, 1);

      vw.finish_example(examples);
    }

    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_spanner_with_actions_that_are_linear_combinations_of_other_actions)
{
  auto d = 8;
  std::vector<VW::workspace*> vws;

  auto* vw_squarecb = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --random_seed 10 --noconstant ",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_squarecb);

  auto* vw_squarecb_zs = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --random_seed 0 --noconstant ",
      nullptr, false, nullptr, nullptr);

  vws.push_back(vw_squarecb_zs);

  for (auto* vw_ptr : vws)
  {
    auto& vw = *vw_ptr;
    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
    }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
    BOOST_CHECK_EQUAL(action_space != nullptr, true);
    action_space->explore._populate_all_testing_components();

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));

      examples.push_back(VW::read_example(vw, "| a_1:0.1 a_2:0.25 a_3:0.12 a100:1 a200:0.1"));
      examples.push_back(VW::read_example(vw, "| a_1:0.2 a_2:0.32 a_3:0.15 a100:0.2 a200:0.2"));
      // linear combination of the above two actions
      // action_4 = action_2 + 2 * action_3
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.89 a_3:0.42 a100:1.4 a200:0.5"));

      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10: 0.2"));
      examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9 v1:0.99"));
      examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18:0.2"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    // we only really care to explore as many actions as there are non degenerate singular values
    // after that actions aren't going to be diverse enough so the spanner will pick similar actions

    // in this case one of the actions that is picked in the spanner is the linear combination of the two other actions,
    // and those two actions do not get picked by the spanner

    auto non_degenerate_singular_values = action_space->explore.number_of_non_degenerate_singular_values();
    action_space->explore._test_only_set_rank(non_degenerate_singular_values);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));

      examples.push_back(VW::read_example(vw, "| a_1:0.1 a_2:0.25 a_3:0.12 a100:1 a200:0.1"));
      examples.push_back(VW::read_example(vw, "| a_1:0.2 a_2:0.32 a_3:0.15 a100:0.2 a200:0.2"));
      // linear combination of the above two actions
      // action_4 = action_2 + 2 * action_3
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.89 a_3:0.42 a100:1.4 a200:0.5"));

      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10: 0.2"));
      examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9 v1:0.99"));
      examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18:0.2"));

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;

      size_t encounters = 0;
      bool action_4_in_spanner = false;
      for (auto& a_s : preds)
      {
        if (a_s.action == 1 && a_s.score != 0.f) { encounters++; }
        if (a_s.action == 2 && a_s.score != 0.f) { encounters++; }
        if (a_s.action == 3 && a_s.score != 0.f)
        {
          encounters++;
          action_4_in_spanner = true;
        }
      }

      BOOST_CHECK_EQUAL(encounters, 1);
      BOOST_CHECK_EQUAL(action_4_in_spanner, true);

      vw.finish_example(examples);
    }

    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_singular_value_sum_diff_for_diff_ranks_is_small)
{
  // d actions with larger values (factor of 10x)
  // 10d - d (the rest) actions with smaller values
  // expect the d actions to be chosen by the spanner

  auto d = 5;
  auto K = 10 * d;

  auto exs = gen_cb_examples(K - d, 10, 1.f);
  auto dexs = gen_cb_examples(d, 10, 100.f, false);

  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 12", nullptr,
      false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    for (auto ex : dexs) { examples.push_back(VW::read_example(vw, ex)); }
    for (auto ex : exs) { examples.push_back(VW::read_example(vw, ex)); }

    vw.learn(examples);
    vw.finish_example(examples);
  }

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  {
    BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
  }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();
  BOOST_CHECK_EQUAL(action_space != nullptr, true);
  action_space->explore._populate_all_testing_components();

  float small_rank_sum = 0;
  float larger_rank_sum = 0;

  {
    action_space->explore._test_only_set_rank(d);
    VW::multi_ex examples;

    for (auto ex : dexs) { examples.push_back(VW::read_example(vw, ex)); }
    for (auto ex : exs) { examples.push_back(VW::read_example(vw, ex)); }

    vw.predict(examples);

    small_rank_sum = action_space->explore.S.sum();
    vw.finish_example(examples);
  }

  {
    action_space->explore._test_only_set_rank(d + 10);
    VW::multi_ex examples;

    for (auto ex : dexs) { examples.push_back(VW::read_example(vw, ex)); }
    for (auto ex : exs) { examples.push_back(VW::read_example(vw, ex)); }

    vw.predict(examples);

    larger_rank_sum = action_space->explore.S.sum();
    vw.finish_example(examples);
  }

  BOOST_CHECK_SMALL(small_rank_sum - larger_rank_sum, 100.f);

  VW::finish(vw);
}

BOOST_AUTO_TEST_SUITE_END()