#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include "learner.h"
#include "pmf_to_pdf.h"
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

namespace VW { namespace pmf_to_pdf {

void learn(VW::pmf_to_pdf::pdf_data& data, single_learner& base, example& ec);
void predict(VW::pmf_to_pdf::pdf_data& data, single_learner& base, example& ec);

struct reduction_test_harness
{
  reduction_test_harness() : _curr_idx(0) {}

  void set_predict_response(const vector<float>& predictions) { _predictions = predictions; }
  void set_chosen_action(const cb_triple& chosen_action) { _action = chosen_action; }

  void test_predict(single_learner& base, example& ec)
  {
    ec.pred.a_s.clear();
    for (uint32_t i = 0; i < _predictions.size(); i++)
    {
      ec.pred.a_s.push_back(ACTION_SCORE::action_score{i, _predictions[i]});
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
    ec.l.cb_cont.costs.push_back(VW::cb_continuous::cb_cont_class{_action.cost, _action.action, _action.prob, 0.f});
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
  vector<float> _predictions;
  cb_triple _action;
  int _curr_idx;
};

using test_learner_t = learner<reduction_test_harness, example>;
using predictions_t = vector<float>;

test_learner_t* get_test_harness_reduction(
    const predictions_t& base_reduction_predictions, const cb_triple& action_triple);

}  // namespace pmf_to_pdf
}  // namespace VW

BOOST_AUTO_TEST_CASE(continuous_action_basic)
{
  uint32_t k = 4;
  uint32_t h = 1;
  float min_val = 1000;
  float max_val = 1100;

  cb_triple action_triple;
  action_triple.set_action(1010.17f, 0.5f, 0.1f);
  VW::pmf_to_pdf::predictions_t prediction_scores;
  prediction_scores = {0.25f, 0.25f, 0.25f, 0.25f};

  const auto test_harness = VW::pmf_to_pdf::get_test_harness_reduction(prediction_scores, action_triple);

  example ec;
  ec.pred.a_s = v_init<ACTION_SCORE::action_score>();
  ec.l.cb.costs = v_init<CB::cb_class>();

  auto data = scoped_calloc_or_throw<VW::pmf_to_pdf::pdf_data>();
  data->set_num_actions(k);
  data->set_bandwidth(h);
  data->set_min_value(min_val);
  data->set_max_value(max_val);

  VW::pmf_to_pdf::predict(*data, *as_singleline(test_harness), ec);

  float sum = 0;
  cout << "ec.pred.p_d (PDF): " << endl;
  for (uint32_t i = 0; i < k; i++)
  {
    cout << "(" << ec.pred.prob_dist[i].action << " : " << ec.pred.prob_dist[i].value << "), " << endl;
    sum += ec.pred.prob_dist[i].value;
  }
  cout << "sum = " << sum << endl;

  VW::pmf_to_pdf::learn(*data, *as_singleline(test_harness), ec);

  cout << "ec.l.cb.costs after:" << endl;
  for (uint32_t i = 0; i < ec.l.cb.costs.size(); i++)
  {
    cout << "(" << ec.l.cb.costs[i].action << " , " << ec.l.cb.costs[i].cost << " , " << ec.l.cb.costs[i].probability
         << " , " << ec.l.cb.costs[i].partial_prediction << "), " << endl;
  }
  cout << "here" << endl;
}

namespace VW { namespace pmf_to_pdf {
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
