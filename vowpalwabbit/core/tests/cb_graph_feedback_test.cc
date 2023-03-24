// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/random.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_graph_feedback.h"
#include "vw/core/vw.h"
#include "vw/test_common/matchers.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

using namespace testing;
constexpr float EXPLICIT_FLOAT_TOL = 0.01f;

std::vector<std::vector<float>> predict_learn_return_action_scores_two_actions(
    VW::workspace& vw, const std::string& shared_graph)
{
  std::vector<std::vector<float>> result;

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, shared_graph + " | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "0:0.2:0.4 | a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "| a_2 b_2 c_2"));

    vw.learn(examples);
    vw.predict(examples);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, shared_graph + " | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "| a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "0:0.8:0.4 | a_2 b_2 c_2"));

    vw.learn(examples);
    vw.predict(examples);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.finish_example(examples);
  }

  return result;
}

TEST(GraphFeedback, CopsAndRobbers)
{
  // aka one reveals info about the other so just give higher probability to the one with the lower cost

  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma", "25"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 0 1
   * 1 0
   */
  std::string shared_graph = "shared graph 0,0,0 0,1,1 1,0,1 1,1,0";

  auto pred_results = predict_learn_return_action_scores_two_actions(vw, shared_graph);

  // f_hat 0.1998, 0.0999 -> second one has lower cost
  EXPECT_THAT(pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.042, 0.957}));

  // fhat 0.4925, 0.6972 -> first one has lower cost
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.913, 0.080}));
}

// TODO des an mporeis na vgaleis tous arithmous apo to peiper etsi opws dinontai
TEST(GraphFeedback, AppleTasting)
{
  // aka spam filtering, or, one action reveals all and the other action reveals nothing

  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma", "25"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 0 1
   * 0 1
   */
  std::string shared_graph = "shared graph 0,0,0 0,1,1 1,0,0 1,1,1";

  auto pred_results = predict_learn_return_action_scores_two_actions(vw, shared_graph);

  // f_hat 0.1998, 0.0999 -> just pick the one with the lowest cost since it also reveals all
  EXPECT_THAT(pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 1.0}));

  // fhat 0.4925, 0.6972 -> the one that reveals all has the higher cost so give it some probability but not the biggest
  // -> the bigger the gamma the more we go with the scores, the smaller the gamma the more we go with the graph
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.940, 0.0599}));
}

TEST(GraphFeedback, PostedPriceAuctionBidding)
{
  // Two actions, one is "optional" (here action 1) and the other is "do nothing" (action 0). The value of action
  // 0 is always revealed but the value of action 1 is revealed only if action 1 is taken.
  // If the "optional" (action 1) action has the lower estimated loss/cost, then we can simply play it with probability
  // 1 If the "do nothing" (action 0) action has the lower estimated loss/cost then p[0] = gamma * f_hat[1] / (1 + gamma
  // * f_hat[1]) + (upper bound of another value)

  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma", "2"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 1 1
   * 0 1
   */
  std::string shared_graph = "shared graph 0,0,1 0,1,1 1,0,0 1,1,1";

  auto pred_results = predict_learn_return_action_scores_two_actions(vw, shared_graph);

  // f_hat 0.1998, 0.0999
  EXPECT_THAT(pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 1.0}));

  // fhat 0.4925, 0.6972
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.722, 0.277}));
}

std::vector<std::vector<float>> predict_learn_return_as(VW::workspace& vw, const std::string& shared_graph)
{
  std::vector<std::vector<float>> result;

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, shared_graph + " | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "0:0.8:0.4 | a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "| a_2 b_2 c_2"));
    examples.push_back(VW::read_example(vw, "| a_100"));

    vw.predict(examples);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.learn(examples);
    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, shared_graph + " | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "| b_1 c_1 d_1"));
    examples.push_back(VW::read_example(vw, "0:0.1:0.4 | b_2 c_2 d_2"));
    examples.push_back(VW::read_example(vw, "| a_100"));

    vw.predict(examples);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.learn(examples);
    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, shared_graph + " | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "0:0.8:0.4 | a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "| a_2 b_2 c_2"));
    examples.push_back(VW::read_example(vw, "| a_100"));

    vw.predict(examples);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.learn(examples);
    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, shared_graph + " | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "| a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "| a_3 b_3 c_3"));
    examples.push_back(VW::read_example(vw, "| a_100"));

    vw.predict(examples);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.learn(examples);
    vw.finish_example(examples);
  }

  return result;
}

TEST(GraphFeedback, CheckIdentityG)
{
  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 1 0 0
   * 0 1 0
   * 0 0 1
   */
  std::string shared_graph = "shared graph 0,0,1 0,1,0 0,2,0 1,0,0 1,1,1 1,2,0 2,0,0 2,1,0 2,2,1";

  auto pred_results = predict_learn_return_as(vw, shared_graph);

  // f_hat 0, 0, 0
  EXPECT_THAT(
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.333, 0.333, 0.333}));

  // fhat 0.5018 0.3011 0.3011
  EXPECT_THAT(
      pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.062, 0.468, 0.468}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.060, 0.530, 0.409}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(
      pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.054, 0.472, 0.472}));
}

TEST(GraphFeedback, CheckLastCol1G)
{
  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 0 0 1
   * 0 0 1
   * 0 0 1
   */
  std::string shared_graph = "shared graph 0,0,0 0,1,0 0,2,1 1,0,0 1,1,0 1,2,1 2,0,0 2,1,0 2,2,1";

  auto pred_results = predict_learn_return_as(vw, shared_graph);

  // f_hat 0, 0, 0
  EXPECT_THAT(
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.333, 0.333, 0.333}));

  // fhat 0.5018 0.3011 0.3011
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.5, 0.5}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.574, 0.426}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.5, 0.5}));
}

TEST(GraphFeedback, CheckFirstCol1G)
{
  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 1 0 0
   * 1 0 0
   * 1 0 0
   */
  std::string shared_graph = "shared graph 0,0,1 0,1,0 0,2,0 1,0,1 1,1,0 1,2,0 2,0,1 2,1,0 2,2,0";

  auto pred_results = predict_learn_return_as(vw, shared_graph);

  // f_hat 0, 0, 0
  EXPECT_THAT(
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.333, 0.333, 0.333}));

  // fhat 0.5018 0.3011 0.3011 -> 1.31
  EXPECT_THAT(
      pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.083, 0.458, 0.458}));

  // fhat 0.5640 0.1585 0.2629 -> 1.21
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.081, 0.514, 0.403}));

  // 0.7371 0.3482 0.3482 -> 1.37
  EXPECT_THAT(
      pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.074, 0.462, 0.462}));
}

TEST(GraphFeedback, CheckAll1sG)
{
  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 1 1 1
   * 1 1 1
   * 1 1 1
   */
  std::string shared_graph = "shared graph 0,0,1 0,1,1 0,2,1 1,0,1 1,1,1 1,2,1 2,0,1 2,1,1 2,2,1";

  auto pred_results = predict_learn_return_as(vw, shared_graph);

  // f_hat 0, 0, 0
  EXPECT_THAT(
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.333, 0.333, 0.333}));

  // fhat 0.5018 0.3011 0.3011 -> 1.31
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.5, 0.5}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.574, 0.426}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.5, 0.5}));
}
