// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "learner.h"
#include "pmf_to_pdf.h"
#include "action_score.h"
#include "cb_label_parser.h"

using namespace VW::LEARNER;
using std::cout;
using std::endl;
using std::pair;
using std::vector;

namespace VW
{
namespace pmf_to_pdf
{
void learn(VW::pmf_to_pdf::reduction& data, single_learner& base, example& ec);
void predict(VW::pmf_to_pdf::reduction& data, single_learner& base, example& ec);

struct reduction_test_harness
{
  reduction_test_harness() : _curr_idx(0) {}

  void set_predict_response(const vector<pair<uint32_t, float>>& predictions) { _predictions = predictions; }

  void test_predict(base_learner& base, example& ec)
  {
    ec.pred.a_s.clear();
    for (uint32_t i = 0; i < _predictions.size(); i++)
    { ec.pred.a_s.push_back(ACTION_SCORE::action_score{_predictions[i].first, _predictions[i].second}); }
  }

  void test_learn(base_learner& base, example& ec)
  { /*noop*/
  }

  static void predict(reduction_test_harness& test_reduction, base_learner& base, example& ec)
  {
    test_reduction.test_predict(base, ec);
  }

  static void learn(reduction_test_harness& test_reduction, base_learner& base, example& ec)
  {
    test_reduction.test_learn(base, ec);
  };

private:
  vector<pair<uint32_t, float>> _predictions;
  int _curr_idx;
};

using test_learner_t = learner<reduction_test_harness, example>;
using predictions_t = vector<pair<uint32_t, float>>;

test_learner_t* get_test_harness_reduction(const predictions_t& base_reduction_predictions);

}  // namespace pmf_to_pdf
}  // namespace VW

void check_pdf_sums_to_one(VW::continuous_actions::probability_density_function& pdf)
{
  float sum = 0;
  for (uint32_t i = 0; i < pdf.size(); i++) { sum += pdf[i].pdf_value * (pdf[i].right - pdf[i].left); }

  BOOST_CHECK_CLOSE(1.0f, sum, .0001f);
}

void check_pdf_limits_are_valid(VW::continuous_actions::probability_density_function& pdf, float min_value,
    float max_value, float bandwidth, uint32_t num_actions, uint32_t action)
{
  // check that left <= right for all pdf
  float prev_pdf_limit = 0;
  for (uint32_t i = 0; i < pdf.size(); i++)
  {
    BOOST_CHECK_GE(pdf[i].left, prev_pdf_limit);
    BOOST_CHECK_GE(pdf[i].right, pdf[i].left);
    prev_pdf_limit = pdf[i].right;
  }

  // check that leftmost >= min_value and rightmost <= max_value
  BOOST_CHECK_GE(pdf[0].left, min_value);
  BOOST_CHECK_LE(pdf[pdf.size() - 1].right, max_value);

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
      if (pdf[i].left == 0 || pdf[i].right == num_actions - 1)
      { BOOST_CHECK_LT(right_unit - left_unit, 2 * bandwidth); }
      else
      {
        BOOST_CHECK_EQUAL(right_unit - left_unit, 2 * bandwidth);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(pmf_to_pdf_basic)
{
  uint32_t k = 4;
  float h = 10.f;  // h (bandwidth) property of continuous range (max_val - min_val)
  float min_val = 1000;
  float max_val = 1100;

  uint32_t action = 2;
  const VW::pmf_to_pdf::predictions_t prediction_scores{{action, 1.f}};

  const auto test_harness = VW::pmf_to_pdf::get_test_harness_reduction(prediction_scores);

  example ec;

  auto data = scoped_calloc_or_throw<VW::pmf_to_pdf::reduction>();
  data->num_actions = k;
  data->bandwidth = h;
  data->min_value = min_val;
  data->max_value = max_val;
  data->_p_base = as_singleline(test_harness);

  predict(*data, *data->_p_base, ec);

  check_pdf_sums_to_one(ec.pred.pdf);
  check_pdf_limits_are_valid(ec.pred.pdf, min_val, max_val, h, k, action);

  ec.l.cb_cont.costs.clear();
  ec.l.cb_cont.costs.push_back({1010.17f, .5f, .05f});  // action, cost, prob

  VW::cb_continuous::continuous_label_elm exp_val{1010.17f, 0.5f, 0.05f};

  BOOST_CHECK_EQUAL(1010.17f, ec.l.cb_cont.costs[0].action);
  BOOST_CHECK_EQUAL(0.5f, ec.l.cb_cont.costs[0].cost);
  BOOST_CHECK_EQUAL(0.05f, ec.l.cb_cont.costs[0].pdf_value);

  learn(*data, *as_singleline(test_harness), ec);

  test_harness->finish();
  delete test_harness;
}

BOOST_AUTO_TEST_CASE(pmf_to_pdf_w_large_bandwidth)
{
  example ec;
  auto data = scoped_calloc_or_throw<VW::pmf_to_pdf::reduction>();
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
    const VW::pmf_to_pdf::predictions_t prediction_scores{{action, 1.f}};

    const auto test_harness = VW::pmf_to_pdf::get_test_harness_reduction(prediction_scores);
    data->_p_base = as_singleline(test_harness);

    predict(*data, *data->_p_base, ec);

    check_pdf_sums_to_one(ec.pred.pdf);
    check_pdf_limits_are_valid(ec.pred.pdf, min_val, max_val, h, k, action);

    test_harness->finish();
    delete test_harness;
  }
}

BOOST_AUTO_TEST_CASE(pmf_to_pdf_w_large_discretization)
{
  example ec;
  auto data = scoped_calloc_or_throw<VW::pmf_to_pdf::reduction>();

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
    const VW::pmf_to_pdf::predictions_t prediction_scores{{action, 1.f}};

    const auto test_harness = VW::pmf_to_pdf::get_test_harness_reduction(prediction_scores);
    data->_p_base = as_singleline(test_harness);

    predict(*data, *data->_p_base, ec);

    check_pdf_sums_to_one(ec.pred.pdf);
    check_pdf_limits_are_valid(ec.pred.pdf, min_val, max_val, h, k, action);

    test_harness->finish();
    delete test_harness;
  }
}

namespace VW
{
namespace pmf_to_pdf
{
test_learner_t* get_test_harness_reduction(const predictions_t& base_reduction_predictions)
{
  // Setup a test harness base reduction
  auto test_harness = VW::make_unique<reduction_test_harness>();
  test_harness->set_predict_response(base_reduction_predictions);
  auto test_learner = VW::LEARNER::make_base_learner(
      std::move(test_harness),          // Data structure passed by vw_framework into test_harness predict/learn calls
      reduction_test_harness::learn,    // test_harness learn
      reduction_test_harness::predict,  // test_harness predict
      "test_learner", prediction_type_t::action_scores, label_type_t::continuous)
                          .build();  // Create a learner using the base reduction.
  return test_learner;
}
}  // namespace pmf_to_pdf
}  // namespace VW
