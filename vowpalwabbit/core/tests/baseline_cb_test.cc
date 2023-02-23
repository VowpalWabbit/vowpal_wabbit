// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/random_details.h"
#include "vw/core/learner.h"
#include "vw/core/metric_sink.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ios>
#include <sstream>
#include <vector>

namespace test_helpers
{
void make_example(VW::multi_ex& examples, VW::workspace& vw, int arm, float* costs, float* probs)
{
  examples.push_back(VW::read_example(vw, "shared | shared_f"));
  for (int i = 0; i < 4; ++i)
  {
    std::stringstream str;
    if (i == arm) { str << "0:" << std::fixed << costs[i] << ":" << probs[i] << " "; }
    str << "| "
        << "arm_" << i;
    examples.push_back(VW::read_example(vw, str.str().c_str()));
  }
}

int sample(int size, const float* probs, float s)
{
  for (int i = 0; i < size; ++i)
  {
    if (s <= probs[i]) { return i; }
    s -= probs[i];
  }
  return 0;  // error
}

}  // namespace test_helpers

TEST(BaselineCB, BaselinePerformsBadly)
{
  using namespace test_helpers;
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--baseline_challenger_cb", "--quiet",
      "--extra_metrics", "ut_metrics.json", "--random_seed", "5"));
  float costs_p0[] = {-0.1f, -0.3f, -0.3f, -1.0f};
  float probs_p0[] = {0.05f, 0.05f, 0.05f, 0.85f};

  uint64_t state = 37;
  for (int i = 0; i < 50; ++i)
  {
    float s = VW::details::merand48(state);
    VW::multi_ex ex;

    make_example(ex, *vw, sample(4, probs_p0, s), costs_p0, probs_p0);
    vw->learn(ex);
    vw->finish_example(ex);
  }

  auto metrics = vw->global_metrics.collect_metrics(vw->l.get());

  EXPECT_EQ(metrics.get_bool("baseline_cb_baseline_in_use"), false);
  // if baseline is not in use, it means the CI lower bound is smaller than the policy expectation
  EXPECT_LE(metrics.get_float("baseline_cb_baseline_lowerbound"), metrics.get_float("baseline_cb_policy_expectation"));

  VW::multi_ex tst;
  make_example(tst, *vw, -1, costs_p0, probs_p0);
  vw->predict(tst);
  EXPECT_EQ(tst[0]->pred.a_s.size(), 4);
  EXPECT_EQ(tst[0]->pred.a_s[0].action, 3);
  EXPECT_GE(tst[0]->pred.a_s[0].score, 0.9625f);  // greedy action, 4 actions + egreedy 0.05

  vw->finish_example(tst);
}

TEST(BaselineCB, BaselineTakesOverPolicy)
{
  using namespace test_helpers;
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--baseline_challenger_cb", "--cb_c_tau", "0.995",
      "--quiet", "--power_t", "0", "-l", "0.001", "--extra_metrics", "ut_metrics.json", "--random_seed", "5"));
  float costs_p0[] = {-0.1f, -0.3f, -0.3f, -1.0f};
  float probs_p0[] = {0.05f, 0.05f, 0.05f, 0.85f};

  float costs_p1[] = {-1.0f, -0.3f, -0.3f, -0.1f};
  float probs_p1[] = {0.05f, 0.05f, 0.05f, 0.85f};

  uint64_t state = 37;
  for (int i = 0; i < 500; ++i)
  {
    float s = VW::details::merand48(state);
    VW::multi_ex ex;

    make_example(ex, *vw, sample(4, probs_p0, s), costs_p0, probs_p0);
    vw->learn(ex);
    vw->finish_example(ex);
  }

  for (int i = 0; i < 400; ++i)
  {
    float s = VW::details::merand48(state);
    VW::multi_ex ex;

    make_example(ex, *vw, sample(4, probs_p1, s), costs_p1, probs_p1);
    vw->learn(ex);
    vw->finish_example(ex);
  }

  // after 400 steps of switched reward dynamics, the baseline CI should have caught up.
  auto metrics = vw->global_metrics.collect_metrics(vw->l.get());

  EXPECT_EQ(metrics.get_bool("baseline_cb_baseline_in_use"), true);

  // if baseline is not in use, it means the CI lower bound is smaller than the policy expectation
  EXPECT_GT(metrics.get_float("baseline_cb_baseline_lowerbound"), metrics.get_float("baseline_cb_policy_expectation"));

  VW::multi_ex tst;
  make_example(tst, *vw, -1, costs_p1, probs_p1);
  vw->predict(tst);

  EXPECT_EQ(tst[0]->pred.a_s.size(), 4);
  EXPECT_EQ(tst[0]->pred.a_s[0].action, 0);
  EXPECT_GE(tst[0]->pred.a_s[0].score, 0.9625f);  // greedy action, 4 actions + egreedy 0.05

  vw->finish_example(tst);
}

VW::metric_sink run_simulation(int steps, int switch_step)
{
  using namespace test_helpers;
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--baseline_challenger_cb", "--quiet",
      "--extra_metrics", "ut_metrics.json", "--random_seed", "5"));
  float costs_p0[] = {-0.1f, -0.3f, -0.3f, -1.0f};
  float probs_p0[] = {0.05f, 0.05f, 0.05f, 0.85f};

  uint64_t state = 37;

  for (int i = 0; i < steps; ++i)
  {
    float s = VW::details::merand48(state);
    VW::multi_ex ex;

    make_example(ex, *vw, sample(4, probs_p0, s), costs_p0, probs_p0);
    vw->learn(ex);
    vw->finish_example(ex);
    if (i == switch_step)
    {
      VW::save_predictor(*vw, "model_file.vw");
      vw = VW::initialize(vwtest::make_args("--quiet", "--extra_metrics", "ut_metrics.json", "-i", "model_file.vw"));
    }
  }
  auto metrics = vw->global_metrics.collect_metrics(vw->l.get());
  return metrics;
}

TEST(BaselineCB, SaveLoadTest)
{
  using namespace test_helpers;

  auto m1 = run_simulation(50, -1);
  auto m2 = run_simulation(50, 20);

  EXPECT_EQ(m1.get_bool("baseline_cb_baseline_in_use"), m2.get_bool("baseline_cb_baseline_in_use"));
  EXPECT_EQ(m1.get_float("baseline_cb_baseline_lowerbound"), m2.get_float("baseline_cb_baseline_lowerbound"));
  EXPECT_EQ(m1.get_float("baseline_cb_policy_expectation"), m2.get_float("baseline_cb_policy_expectation"));
}
