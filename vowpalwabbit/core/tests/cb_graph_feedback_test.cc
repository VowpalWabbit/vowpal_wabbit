// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "simulator.h"
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

using simulator::callback_map;
using simulator::cb_sim;

// Small gamma -> graph respected / High gamma -> costs respected

void check_probs_sum_to_one(const VW::action_scores& action_scores)
{
  float sum = 0;
  for (auto& action_score : action_scores) { sum += action_score.score; }
  EXPECT_NEAR(sum, 1, EXPLICIT_FLOAT_TOL);
}

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

    check_probs_sum_to_one(examples[0]->pred.a_s);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, shared_graph + " | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "| a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "1:0.8:0.4 | a_2 b_2 c_2"));

    vw.learn(examples);
    vw.predict(examples);
    check_probs_sum_to_one(examples[0]->pred.a_s);

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

  // gamma = gamma_scale * count ^ gamma_exponent
  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "10", "--gamma_exponent", "0"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  auto& vw = *vw_graph.get();

  /**
   * 0 1
   * 1 0
   */
  std::string shared_graph = "shared graph 0,0,0 0,1,1 1,0,1 1,1,0";

  auto pred_results = predict_learn_return_action_scores_two_actions(vw, shared_graph);

  // f_hat 0.1998, 0.0999 -> second one has lower cost
  EXPECT_THAT(pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.099, 0.900}));

  // fhat 0.4925, 0.6972 -> first one has lower cost
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.710, 0.289}));
}

TEST(GraphFeedback, AppleTasting)
{
  // aka spam filtering, or, one action reveals all and the other action reveals nothing

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "10", "--gamma_exponent", "0"};
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
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.799, 0.20}));
}

TEST(GraphFeedback, PostedPriceAuctionBidding)
{
  // Two actions, one is "optional" (here action 1) and the other is "do nothing" (action 0). The value of action
  // 0 is always revealed but the value of action 1 is revealed only if action 1 is taken.
  // If the "optional" (action 1) action has the lower estimated loss/cost, then we can simply play it with probability
  // 1 If the "do nothing" (action 0) action has the lower estimated loss/cost then p[0] = gamma * f_hat[1] / (1 + gamma
  // * f_hat[1]) + (upper bound of another value)

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "10", "--gamma_exponent", "0"};
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
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.834, 0.165}));
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

    check_probs_sum_to_one(examples[0]->pred.a_s);

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
    examples.push_back(VW::read_example(vw, "1:0.1:0.4 | b_2 c_2 d_2"));
    examples.push_back(VW::read_example(vw, "| a_100"));

    vw.predict(examples);

    check_probs_sum_to_one(examples[0]->pred.a_s);

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

    check_probs_sum_to_one(examples[0]->pred.a_s);

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

    check_probs_sum_to_one(examples[0]->pred.a_s);

    std::vector<float> scores(examples[0]->pred.a_s.size());
    for (auto& action_score : examples[0]->pred.a_s) { scores[action_score.action] = action_score.score; }

    result.push_back(scores);

    vw.learn(examples);
    vw.finish_example(examples);
  }

  return result;
}

TEST(GraphFeedback, CheckIdentityGSmallGamma)
{
  // With the identity graph we just go with the cost i.e. highest cost -> lowest probability
  // You can see it respecting the costs as gamma increases and the graph losses its power in the decision making

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "1", "--gamma_exponent", "0"};
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
      pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.288, 0.355, 0.355}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.236, 0.403, 0.360}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(
      pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.245, 0.377, 0.377}));
}

TEST(GraphFeedback, CheckIdentityGLargeGamma)
{
  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "20", "--gamma_exponent", "0"};
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
      pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.052, 0.473, 0.473}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.039, 0.909, 0.051}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(
      pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.044, 0.477, 0.477}));
}

TEST(GraphFeedback, CheckLastCol1GSmallGamma)
{
  // the last action reveals everything about everything, the other two actions don't reveal anything
  // it does take the cost into account but the weight of the probabilities should lie towards the last action

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "1", "--gamma_exponent", "0"};
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
  EXPECT_THAT(pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.0, 1}));

  // fhat 0.5018 0.3011 0.3011
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.0, 1}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.0, 1}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.0, 1}));
}

TEST(GraphFeedback, CheckLastCol1GMedGamma)
{
  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "2.5", "--gamma_exponent", "0"};
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
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.171, 0.171, 0.656}));

  // fhat 0.5018 0.3011 0.3011
  EXPECT_THAT(
      pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.061, 0.230, 0.708}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.118, 0.449, 0.432}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0, 0.305, 0.694}));
}

TEST(GraphFeedback, CheckLastCol1GLargeGamma)
{
  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "10", "--gamma_exponent", "0"};
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
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.322, 0.322, 0.354}));

  // fhat 0.5018 0.3011 0.3011
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0, 0.5, 0.5}));

  // fhat 0.5640 0.1585 0.2629
  EXPECT_THAT(pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0, 0.582, 0.417}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0, 0.5, 0.5}));
}

TEST(GraphFeedback, CheckFirstCol1GSmallGamma)
{
  // now the probs should favour the first action

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "1", "--gamma_exponent", "0"};
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
  EXPECT_THAT(pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{1, 0, 0}));

  // fhat 0.5018 0.3011 0.3011 -> 1.31
  EXPECT_THAT(pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{1, 0, 0}));

  // fhat 0.5640 0.1585 0.2629 -> 1.21
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.646, 0.186, 0.167}));

  // 0.7371 0.3482 0.3482 -> 1.37
  EXPECT_THAT(pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{1, 0, 0}));
}

TEST(GraphFeedback, CheckFirstCol1GMedGamma)
{
  // now the probs should favour the first action

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "2.5", "--gamma_exponent", "0"};
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
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.656, 0.171, 0.171}));

  // fhat 0.5018 0.3011 0.3011 -> 1.31
  EXPECT_THAT(
      pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.426, 0.286, 0.286}));

  // fhat 0.5640 0.1585 0.2629 -> 1.21
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.305, 0.366, 0.328}));

  // 0.7371 0.3482 0.3482 -> 1.37
  EXPECT_THAT(
      pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.435, 0.282, 0.282}));
}

TEST(GraphFeedback, CheckFirstCol1GLargeGamma)
{
  // now the probs should favour the first action

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "100", "--gamma_exponent", "0"};
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
      pred_results[0], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.354, 0.322, 0.322}));

  // fhat 0.5018 0.3011 0.3011 -> 1.31
  EXPECT_THAT(
      pred_results[1], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.016, 0.491, 0.4919}));

  // fhat 0.5640 0.1585 0.2629 -> 1.21
  EXPECT_THAT(
      pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.011, 0.575, 0.412}));

  // 0.7371 0.3482 0.3482 -> 1.37
  EXPECT_THAT(
      pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.014, 0.492, 0.492}));
}

TEST(GraphFeedback, CheckSupervisedG)
{
  // if they are all 1s that means that all reveal information for all

  std::vector<std::string> args{
      "--cb_explore_adf", "--graph_feedback", "--quiet", "--gamma_scale", "10", "--gamma_exponent", "0"};
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
  EXPECT_THAT(pred_results[2], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0, 0.579, 0.420}));

  // 0.7371 0.3482 0.3482
  EXPECT_THAT(pred_results[3], testing::Pointwise(FloatNear(EXPLICIT_FLOAT_TOL), std::vector<float>{0.0, 0.5, 0.5}));
}

TEST(GraphFeedback, CheckUpdateRule100WIterations)
{
  callback_map test_hooks;

  std::vector<std::string> vw_arg{"--cb_explore_adf", "--quiet", "--random_seed", "5", "-q", "UA"};

  int seed = 101;
  size_t num_iterations = 100;

  auto vw_arg_gf = vw_arg;
  vw_arg_gf.push_back("--graph_feedback");
  // this is a very simple simulation that converges quickly and small dataset, we want the gamma to grow a bit more
  // aggressively and have the costs dictate the pmf more than the graph
  vw_arg_gf.push_back("--gamma_exponent");
  vw_arg_gf.push_back("1");

  auto vw_gf = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_arg_gf));

  auto vw_arg_egreedy = vw_arg;
  vw_arg_egreedy.push_back("--epsilon");
  vw_arg_egreedy.push_back("0.2");

  auto vw_egreedy = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_arg_egreedy));

  simulator::cb_sim_gf_filtering sim_gf(true, seed);
  simulator::cb_sim_gf_filtering sim_egreedy(false, seed);

  auto ctr_gf = sim_gf.run_simulation_hook(vw_gf.get(), num_iterations, test_hooks);
  auto ctr_egreedy = sim_egreedy.run_simulation_hook(vw_egreedy.get(), num_iterations, test_hooks);

  EXPECT_GT(ctr_gf.back(), ctr_egreedy.back());
  EXPECT_GT(sim_gf.not_spam_classified_as_not_spam, sim_egreedy.not_spam_classified_as_not_spam);
  EXPECT_LT(sim_gf.not_spam_classified_as_spam, sim_egreedy.not_spam_classified_as_spam);
}

TEST(GraphFeedback, CheckUpdateRule500WIterations)
{
  callback_map test_hooks;

  std::vector<std::string> vw_arg{"--cb_explore_adf", "--quiet", "--random_seed", "5", "-q", "UA"};

  int seed = 10;
  size_t num_iterations = 500;

  auto vw_arg_gf = vw_arg;
  vw_arg_gf.push_back("--graph_feedback");
  auto vw_gf = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_arg_gf));

  auto vw_arg_egreedy = vw_arg;
  vw_arg_egreedy.push_back("--epsilon");
  vw_arg_egreedy.push_back("0.2");

  auto vw_egreedy = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_arg_egreedy));

  simulator::cb_sim_gf_filtering sim_gf(true, seed);
  simulator::cb_sim_gf_filtering sim_egreedy(false, seed);

  auto ctr_gf = sim_gf.run_simulation_hook(vw_gf.get(), num_iterations, test_hooks);
  auto ctr_egreedy = sim_egreedy.run_simulation_hook(vw_egreedy.get(), num_iterations, test_hooks);

  EXPECT_GT(ctr_gf.back(), ctr_egreedy.back());
  EXPECT_GT(sim_gf.not_spam_classified_as_not_spam, sim_egreedy.not_spam_classified_as_not_spam);
  EXPECT_LT(sim_gf.not_spam_classified_as_spam, sim_egreedy.not_spam_classified_as_spam);
}