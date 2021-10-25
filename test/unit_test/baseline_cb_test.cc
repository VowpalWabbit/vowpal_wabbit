// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <ios>
#include <sstream>
#include "reductions_fwd.h"

#include "test_common.h"

#include <vector>

#include "vw.h"

namespace test_helpers
{
void make_example(multi_ex& examples, vw& vw, int arm, float* costs, float* probs)
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

BOOST_AUTO_TEST_CASE(baseline_cb_baseline_performs_badly)
{
  using namespace test_helpers;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --baseline_challenger_cb --quiet --extra_metrics ut_metrics.json --random_seed 5");
  float costs_p0[] = {-0.1, -0.3, -0.3, -1.0};
  float probs_p0[] = {0.05, 0.05, 0.05, 0.85};

  uint64_t state = 37;
  for (int i = 0; i < 50; ++i)
  {
    float s = merand48(state);
    multi_ex ex;

    make_example(ex, vw, sample(4, probs_p0, s), costs_p0, probs_p0);
    vw.learn(ex);
    vw.finish_example(ex);
  }

  VW::metric_sink metrics;
  vw.l->persist_metrics(metrics);

  BOOST_CHECK_EQUAL(metrics.get_bool("baseline_cb_baseline_in_use"), false);
  // if baseline is not in use, it means the CI lower bound is smaller than the policy expectation
  BOOST_CHECK_LE(metrics.get_float("baseline_cb_baseline_lowerbound"),
      metrics.get_float("baseline_cb_policy_expectation"));

  multi_ex tst;
  make_example(tst, vw, -1, costs_p0, probs_p0);
  vw.predict(tst);
  BOOST_CHECK_EQUAL(tst[0]->pred.a_s.size(), 4);
  BOOST_CHECK_EQUAL(tst[0]->pred.a_s[0].action, 3);
  BOOST_CHECK_GE(tst[0]->pred.a_s[0].score, 0.9625f);  // greedy action, 4 actions + egreedy 0.05

  vw.finish_example(tst);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(baseline_cb_baseline_takes_over_policy)
{
  using namespace test_helpers;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --baseline_challenger_cb --cb_c_tau 0.995 --quiet --power_t 0 -l 0.001 --extra_metrics "
      "ut_metrics.json --random_seed 5");
  float costs_p0[] = {-0.1, -0.3, -0.3, -1.0};
  float probs_p0[] = {0.05, 0.05, 0.05, 0.85};

  float costs_p1[] = {-1.0, -0.3, -0.3, -0.1};
  float probs_p1[] = {0.05, 0.05, 0.05, 0.85};

  uint64_t state = 37;
  for (int i = 0; i < 500; ++i)
  {
    float s = merand48(state);
    multi_ex ex;

    make_example(ex, vw, sample(4, probs_p0, s), costs_p0, probs_p0);
    vw.learn(ex);
    vw.finish_example(ex);
  }

  for (int i = 0; i < 400; ++i)
  {
    float s = merand48(state);
    multi_ex ex;

    make_example(ex, vw, sample(4, probs_p1, s), costs_p1, probs_p1);
    vw.learn(ex);
    vw.finish_example(ex);
  }

  // after 400 steps of switched reward dynamics, the baseline CI should have caught up.
  VW::metric_sink metrics;
  vw.l->persist_metrics(metrics);

  BOOST_CHECK_EQUAL(metrics.get_bool("baseline_cb_baseline_in_use"), true);

  // if baseline is not in use, it means the CI lower bound is smaller than the policy expectation
  BOOST_CHECK_GT(metrics.get_float("baseline_cb_baseline_lowerbound"),
      metrics.get_float("baseline_cb_policy_expectation"));

  multi_ex tst;
  make_example(tst, vw, -1, costs_p1, probs_p1);
  vw.predict(tst);

  BOOST_CHECK_EQUAL(tst[0]->pred.a_s.size(), 4);
  BOOST_CHECK_EQUAL(tst[0]->pred.a_s[0].action, 0);
  BOOST_CHECK_GE(tst[0]->pred.a_s[0].score, 0.9625f);  // greedy action, 4 actions + egreedy 0.05

  vw.finish_example(tst);

  VW::finish(vw);
}

VW::metric_sink run_simulation(int steps, int switch_step)
{
  using namespace test_helpers;
  auto* vw = VW::initialize(
      "--cb_explore_adf --baseline_challenger_cb --quiet --extra_metrics ut_metrics.json --random_seed 5  "
      "--save_resume");
  float costs_p0[] = {-0.1, -0.3, -0.3, -1.0};
  float probs_p0[] = {0.05, 0.05, 0.05, 0.85};

  uint64_t state = 37;

  for (int i = 0; i < steps; ++i)
  {
    float s = merand48(state);
    multi_ex ex;

    make_example(ex, *vw, sample(4, probs_p0, s), costs_p0, probs_p0);
    vw->learn(ex);
    vw->finish_example(ex);
    if (i == switch_step)
    {
      VW::save_predictor(*vw, "model_file.vw");
      VW::finish(*vw);
      vw = VW::initialize("--quiet --extra_metrics ut_metrics.json --save_resume -i model_file.vw");
    }
  }
  VW::metric_sink metrics;
  vw->l->persist_metrics(metrics);
  VW::finish(*vw);

  return metrics;
}

BOOST_AUTO_TEST_CASE(baseline_cb_save_load_test)
{
  using namespace test_helpers;

  auto m1 = run_simulation(50, -1);
  auto m2 = run_simulation(50, 20);

  BOOST_CHECK_EQUAL(m1.get_bool("baseline_cb_baseline_in_use"), m2.get_bool("baseline_cb_baseline_in_use"));
  BOOST_CHECK_EQUAL(m1.get_float("baseline_cb_baseline_lowerbound"), m2.get_float("baseline_cb_baseline_lowerbound"));
  BOOST_CHECK_EQUAL(m1.get_float("baseline_cb_policy_expectation"), m2.get_float("baseline_cb_policy_expectation"));
}
