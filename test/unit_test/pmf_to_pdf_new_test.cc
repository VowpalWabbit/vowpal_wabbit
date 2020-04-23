#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include "learner.h"
#include "pmf_to_pdf_new.h"
#include "parse_args.h"  // setup_base()
#include "action_score.h"

using namespace LEARNER;
using namespace std;


struct cb_triple
{
  float action;
  float cost;
  float prob;
  void set_action(float x, float y, float z)
  {
    action = x, cost = y; prob = z;
  }
};

namespace VW { namespace pmf_to_pdf_new {

void learn(VW::pmf_to_pdf_new::reduction& data, single_learner& base, example& ec);
void predict(VW::pmf_to_pdf_new::reduction& data, single_learner& base, example& ec);

using test_learner_t = learner<reduction_test_harness, example>;
using predictions_t = vector<pair<int, float>>;

struct reduction_test_harness
{
  reduction_test_harness() : _curr_idx(0) {}

  void set_predict_response(const predictions_t& predictions) { _predictions = predictions; }
  void set_chosen_action(const cb_triple& chosen_action) { _action = chosen_action; }

  void test_predict(single_learner& base, example& ec)
  {
    ec.pred.a_s.clear();
    for (uint32_t i = 0; i < _predictions.size(); i++)
    {
      ec.pred.a_s.push_back(ACTION_SCORE::action_score{_predictions[i].first, _predictions[i].second});
    }

    cout << "\nec.pred.a_s (PMF): " << endl;
    for (uint32_t i = 0; i < _predictions.size(); i++)
    {
      cout << "(" << ec.pred.a_s[i].action << " : " << ec.pred.a_s[i].score << "), " << endl;
    }
  }

  void test_learn(single_learner& base, example& ec)
  {
    ec.l.cb_cont.costs.clear();
    ec.l.cb_cont.costs.push_back(VW::cb_continuous::continuous_label_elm{_action.cost, _action.action, _action.prob, 0.f});
    cout << "ec.l.cb_cont.costs before:" << endl;
    cout << "(" << ec.l.cb_cont.costs[0].action << " , " << ec.l.cb_cont.costs[0].cost << " , " << ec.l.cb_cont.costs[0].probability
         << " , " << ec.l.cb_cont.costs[0].partial_prediction << "), " << endl;
  }

  static void predict(reduction_test_harness& test_reduction, single_learner& base, example& ec)
  {
    test_reduction.test_predict(base, ec);
  }

  static void learn(reduction_test_harness& test_reduction, single_learner& base, example& ec)
  {
    test_reduction.test_learn(base, ec);
  };

 private:
  vector<pair<int, float>> _predictions;
  cb_triple _action;
  int _curr_idx;
};


test_learner_t* get_test_harness_reduction(
    const predictions_t& base_reduction_predictions, const cb_triple& action_triple);


float get_pdf_value(VW::actions_pdf::pdf_new prob_dist_new, float chosen_action)
{
  int begin = -1;
  int end = (int)prob_dist_new.size();
  while (end - begin > 1)
  {
    int mid = (begin + end) / 2;
    if (prob_dist_new[mid].left <= chosen_action)
    {
      begin = mid;
    }
    else
    {
      end = mid;
    }
  }
  // // temporary fix for now
  // if (begin == (int)prob_dist.size() - 1 && prob_dist[begin].value == 0)
  //   return prob_dist[begin - 1].value;
  return prob_dist_new[begin].pdf_value;
}

}  // namespace pmf_to_pdf
}  // namespace VW

BOOST_AUTO_TEST_CASE(pmf_to_pdf_basic)
{
  uint32_t k = 4;
  uint32_t h = 1;
  float min_val = 1000;
  float max_val = 1100;

  cb_triple action_triple;
  action_triple.set_action(1010.17f, 0.5f, 0.1f);
  VW::pmf_to_pdf_new::predictions_t prediction_scores;
  prediction_scores = {{2, 1}};

  const auto test_harness = VW::pmf_to_pdf_new::get_test_harness_reduction(prediction_scores, action_triple);

  example ec;
  ec.pred.a_s = v_init<ACTION_SCORE::action_score>();
  ec.l.cb_cont.costs = v_init<VW::cb_continuous::continuous_label_elm>();

  auto data = scoped_calloc_or_throw<VW::pmf_to_pdf_new::reduction>();
  data->num_actions = k;
  data->bandwidth = h;
  data->min_value = min_val;
  data->max_value = max_val;
  data->_p_base = as_singleline(test_harness);

  predict(*data, *data->_p_base, ec);

  float sum = 0;
  cout << "ec.pred.p_d (PDF): " << endl;
  for (uint32_t i = 0; i < ec.pred.prob_dist_new.size(); i++)
  {
    cout << "(" << ec.pred.prob_dist_new[i].left << ", " << ec.pred.prob_dist_new[i].right << ", " <<
    ec.pred.prob_dist_new[i].pdf_value << ")" << endl;
    sum += ec.pred.prob_dist_new[i].pdf_value * (ec.pred.prob_dist_new[i].right - ec.pred.prob_dist_new[i].left);
  }
  cout << "sum = " << sum << endl;

  ec.l.cb_cont.costs.push_back({1010.0f, .5f, .05f, 0.f});
  learn(*data, *as_singleline(test_harness), ec);

  cout << "ec.l.cb.costs after:" << endl;
  for (uint32_t i = 0; i < ec.l.cb.costs.size(); i++)
  {
    cout << "(" << ec.l.cb.costs[i].action << " , " << ec.l.cb.costs[i].cost << " , " << ec.l.cb.costs[i].probability
         << " , " << ec.l.cb.costs[i].partial_prediction << "), " << endl;
  }

  // float chosen_action = action_triple.action
  float chosen_action = 1080; 
  cout << "pdf value of " << chosen_action << " is = " << VW::pmf_to_pdf_new::get_pdf_value(ec.pred.prob_dist_new, chosen_action)
       << std::endl;
  cout << "here" << endl;
}

namespace VW { namespace pmf_to_pdf_new {
test_learner_t* get_test_harness_reduction(const predictions_t& base_reduction_predictions, const cb_triple& action_triple)
{
  // Setup a test harness base reduction
  auto test_harness = scoped_calloc_or_throw<reduction_test_harness>();
  test_harness->set_predict_response(base_reduction_predictions);
  test_harness->set_chosen_action(action_triple);
  auto& test_learner =
      init_learner(test_harness,          // Data structure passed by vw_framework into test_harness predict/learn calls
          reduction_test_harness::learn,  // test_harness learn
          reduction_test_harness::predict,  // test_harness predict
          1                                 // Number of regressors in test_harness (not used)
      );                                    // Create a learner using the base reduction.
  return &test_learner;
}
}}  // namespace VW::pmf_to_pdf
