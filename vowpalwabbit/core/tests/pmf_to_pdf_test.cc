// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/pmf_to_pdf.h"

#include "vw/core/action_score.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

using namespace VW::LEARNER;
using std::pair;
using std::vector;

namespace
{
class reduction_test_harness
{
public:
  void set_predict_response(const vector<pair<uint32_t, float>>& predictions) { _predictions = predictions; }

  void test_predict(VW::example& ec)
  {
    ec.pred.a_s.clear();
    for (const auto& prediction : _predictions)
    {
      ec.pred.a_s.push_back(VW::action_score{prediction.first, prediction.second});
    }
  }

  void test_learn(VW::example& /* ec */)
  { /*noop*/
  }

  static void predict(reduction_test_harness& test_reduction, VW::example& ec) { test_reduction.test_predict(ec); }

  static void learn(reduction_test_harness& test_reduction, VW::example& ec) { test_reduction.test_learn(ec); };

private:
  vector<pair<uint32_t, float>> _predictions;
};

using predictions_t = vector<pair<uint32_t, float>>;

std::shared_ptr<learner> get_test_harness(const predictions_t& bottom_learner_predictions)
{
  // Setup a test harness bottom learner
  auto test_harness = VW::make_unique<reduction_test_harness>();
  test_harness->set_predict_response(bottom_learner_predictions);
  auto test_learner = VW::LEARNER::make_bottom_learner(
      std::move(test_harness),          // Data structure passed by vw_framework into test_harness predict/learn calls
      reduction_test_harness::learn,    // test_harness learn
      reduction_test_harness::predict,  // test_harness predict
      "test_learner", VW::prediction_type_t::ACTION_SCORES, VW::label_type_t::CONTINUOUS)
                          // Set it to something so that the compat VW::finish_example shim is put in place.
                          .set_output_example_prediction([](VW::workspace& /* all */, const reduction_test_harness&,
                                                             const VW::example&, VW::io::logger&) {})

                          .build();  // Create a learner using the bottom learner.
  return test_learner;
}
}  // namespace

void check_pdf_sums_to_one(VW::continuous_actions::probability_density_function& pdf)
{
  float sum = 0;
  for (uint32_t i = 0; i < pdf.size(); i++) { sum += pdf[i].pdf_value * (pdf[i].right - pdf[i].left); }

  EXPECT_NEAR(1.0f, sum, .0001f);
}

void check_pdf_limits_are_valid(VW::continuous_actions::probability_density_function& pdf, float min_value,
    float max_value, float bandwidth, uint32_t num_actions, uint32_t action)
{
  // check that left <= right for all pdf
  float prev_pdf_limit = 0;
  for (uint32_t i = 0; i < pdf.size(); i++)
  {
    EXPECT_GE(pdf[i].left, prev_pdf_limit);
    EXPECT_GE(pdf[i].right, pdf[i].left);
    prev_pdf_limit = pdf[i].right;
  }

  // check that leftmost >= min_value and rightmost <= max_value
  EXPECT_GE(pdf[0].left, min_value);
  EXPECT_LE(pdf[pdf.size() - 1].right, max_value);

  // check that left/right pair around the predicted action has the right bandwidth
  float continuous_range = max_value - min_value;
  float unit_range = continuous_range / (num_actions - 1);
  for (uint32_t i = 0; i < pdf.size(); i++)
  {
    float left_unit = (pdf[i].left - min_value) / unit_range;
    float right_unit = (pdf[i].right - min_value) / unit_range;
    if (pdf[i].left <= action && action <= pdf[i].right)
    {
      // for 'action' prediced left limit will be 'action - bandwidth', or zero for edge cases where action < bandwidth
      // for 'action' predicted right limit will be 'action + bandwidth',  or 'num_actions - 1' for edge cases
      // where action + bandwidth > num_actions - 1
      // resulting in a span of max 2 * bandwidth
      if (pdf[i].left == 0 || pdf[i].right == num_actions - 1) { EXPECT_LT(right_unit - left_unit, 2 * bandwidth); }
      else { EXPECT_EQ(right_unit - left_unit, 2 * bandwidth); }
    }
  }
}

TEST(PmfToPdf, Basic)
{
  uint32_t k = 4;
  float h = 10.f;  // h (bandwidth) property of continuous range (max_val - min_val)
  float min_val = 1000;
  float max_val = 1100;

  uint32_t action = 2;
  const ::predictions_t prediction_scores{{action, 1.f}};

  const auto test_harness = ::get_test_harness(prediction_scores);

  VW::example ec;

  auto data = VW::make_unique<VW::reductions::pmf_to_pdf_reduction>();
  data->num_actions = k;
  data->bandwidth = h;
  data->min_value = min_val;
  data->max_value = max_val;
  data->_p_base = test_harness.get();

  data->predict(ec);

  check_pdf_sums_to_one(ec.pred.pdf);
  check_pdf_limits_are_valid(ec.pred.pdf, min_val, max_val, h, k, action);

  ec.l.cb_cont.costs.clear();
  ec.l.cb_cont.costs.push_back({1010.17f, .5f, .05f});  // action, cost, prob

  // VW::cb_continuous::continuous_label_elm exp_val{1010.17f, 0.5f, 0.05f};

  EXPECT_EQ(1010.17f, ec.l.cb_cont.costs[0].action);
  EXPECT_EQ(0.5f, ec.l.cb_cont.costs[0].cost);
  EXPECT_EQ(0.05f, ec.l.cb_cont.costs[0].pdf_value);

  data->learn(ec);

  test_harness->finish();
}

TEST(PmfToPdf, WLargeBandwidth)
{
  VW::example ec;
  auto data = VW::make_unique<VW::reductions::pmf_to_pdf_reduction>();
  uint32_t k = 4;   // num_actions
  float h = 300.f;  // // h (bandwidth) property of continuous range (max_val - min_val)
  float min_val = 1000;
  float max_val = 1400;

  // continuous_range = max_value - min_value;
  // unit_range = continuous_range / (k - 1);

  data->num_actions = k;
  data->bandwidth = h;
  data->min_value = min_val;
  data->max_value = max_val;

  for (uint32_t action = 0; action < k; action++)
  {
    const ::predictions_t prediction_scores{{action, 1.f}};

    const auto test_harness = ::get_test_harness(prediction_scores);
    data->_p_base = test_harness.get();

    data->predict(ec);

    check_pdf_sums_to_one(ec.pred.pdf);
    check_pdf_limits_are_valid(ec.pred.pdf, min_val, max_val, h, k, action);

    test_harness->finish();
  }
}

TEST(PmfToPdf, WLargeDiscretization)
{
  VW::example ec;
  auto data = VW::make_unique<VW::reductions::pmf_to_pdf_reduction>();

  uint32_t k = 16;  // num_actions
  float h = 10.f;   // h (bandwidth) property of continuous range (max_val - min_val)
  float min_val = 1000;
  float max_val = 1400;

  data->num_actions = k;
  data->bandwidth = h;
  data->min_value = min_val;
  data->max_value = max_val;

  // continuous_range = max_value - min_value;
  // unit_range = continuous_range / (k - 1);

  for (uint32_t action = 0; action < k; action++)
  {
    const ::predictions_t prediction_scores{{action, 1.f}};

    const auto test_harness = ::get_test_harness(prediction_scores);
    data->_p_base = test_harness.get();

    data->predict(ec);

    check_pdf_sums_to_one(ec.pred.pdf);
    check_pdf_limits_are_valid(ec.pred.pdf, min_val, max_val, h, k, action);

    test_harness->finish();
  }
}
