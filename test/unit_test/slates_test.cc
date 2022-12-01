// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/slates.h"

#include "test_common.h"
#include "vw/core/ccb_label.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/multi_ex.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

template <typename LearnFunc, typename PredictFunc>
class test_base
{
public:
  LearnFunc test_learn_func;
  PredictFunc test_predict_func;

  test_base(LearnFunc learn, PredictFunc predict) : test_learn_func(learn), test_predict_func(predict) {}
  static void invoke_learn(
      test_base<LearnFunc, PredictFunc>& data, VW::LEARNER::base_learner& /*base*/, VW::multi_ex& examples)
  {
    data.test_learn_func(examples);
  }
  static void invoke_predict(
      test_base<LearnFunc, PredictFunc>& data, VW::LEARNER::base_learner& /*base*/, VW::multi_ex& examples)
  {
    data.test_predict_func(examples);
  }
};

template <typename LearnFunc, typename PredictFunc>
VW::LEARNER::learner<test_base<LearnFunc, PredictFunc>, VW::multi_ex>* make_test_learner(
    const LearnFunc& learn, const PredictFunc& predict)
{
  auto test_base_data = VW::make_unique<test_base<LearnFunc, PredictFunc>>(learn, predict);
  using func = void (*)(test_base<LearnFunc, PredictFunc>&, VW::LEARNER::base_learner&, VW::multi_ex&);
  auto learn_fptr = &test_base<LearnFunc, PredictFunc>::invoke_learn;
  auto predict_fptr = &test_base<LearnFunc, PredictFunc>::invoke_predict;
  return VW::LEARNER::make_base_learner(std::move(test_base_data), static_cast<func>(learn_fptr),
      static_cast<func>(predict_fptr), "mock_reduction", VW::prediction_type_t::DECISION_PROBS, VW::label_type_t::CCB)
      // Set it to something so that the compat VW::finish_example shim is put in place.
      .set_output_example(
          [](VW::workspace& all, shared_data& sd, const test_base<LearnFunc, PredictFunc>&, const VW::multi_ex&) {})
      .build();
}

BOOST_AUTO_TEST_CASE(slates_reduction_mock_test)
{
  auto& vw = *VW::initialize("--slates --quiet");
  VW::multi_ex examples;
  examples.push_back(VW::read_example(vw, "slates shared 0.8 | ignore_me"));
  examples.push_back(VW::read_example(vw, "slates action 0 | ignore_me"));
  examples.push_back(VW::read_example(vw, "slates action 1 | ignore_me"));
  examples.push_back(VW::read_example(vw, "slates action 1 | ignore_me"));
  examples.push_back(VW::read_example(vw, "slates slot 0:0.8 | ignore_me"));
  examples.push_back(VW::read_example(vw, "slates slot 1:0.6 | ignore_me"));

  auto mock_learn_or_pred = [](VW::multi_ex& examples) {
    BOOST_CHECK_EQUAL(examples.size(), 6);
    BOOST_CHECK_EQUAL(examples[0]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SHARED);
    BOOST_CHECK_EQUAL(examples[1]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
    BOOST_CHECK_EQUAL(examples[2]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
    BOOST_CHECK_EQUAL(examples[3]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);

    BOOST_CHECK_EQUAL(examples[4]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
    BOOST_CHECK_CLOSE(examples[4]->l.conditional_contextual_bandit.outcome->cost, 0.8f, FLOAT_TOL);
    check_collections_with_float_tolerance(
        examples[4]->l.conditional_contextual_bandit.outcome->probabilities, std::vector<VW::action_score>{{0, 0.8f}});
    check_collections_exact(
        examples[4]->l.conditional_contextual_bandit.explicit_included_actions, std::vector<uint32_t>{0});
    BOOST_CHECK_EQUAL(examples[5]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
    BOOST_CHECK_CLOSE(examples[5]->l.conditional_contextual_bandit.outcome->cost, 0.8f, FLOAT_TOL);
    check_collections_with_float_tolerance(
        examples[5]->l.conditional_contextual_bandit.outcome->probabilities, std::vector<VW::action_score>{{2, 0.6f}});
    check_collections_exact(
        examples[5]->l.conditional_contextual_bandit.explicit_included_actions, std::vector<uint32_t>{1, 2});

    // Prepare and return the prediction
    VW::v_array<VW::action_score> slot_zero;
    slot_zero.push_back({0, 1.0});
    VW::v_array<VW::action_score> slot_one;
    slot_one.push_back({1, 0.5});
    slot_one.push_back({2, 0.5});
    examples[0]->pred.decision_scores.push_back(slot_zero);
    examples[0]->pred.decision_scores.push_back(slot_one);
  };
  auto* test_base_learner = make_test_learner(mock_learn_or_pred, mock_learn_or_pred);
  VW::reductions::slates_data slate_reduction;
  slate_reduction.learn(*VW::LEARNER::as_multiline(test_base_learner), examples);

  // This confirms that the reductions converted the CCB space decision scores back to slates action index space.
  BOOST_CHECK_EQUAL(examples[0]->pred.decision_scores.size(), 2);
  check_collections_with_float_tolerance(examples[0]->pred.decision_scores[0], std::vector<VW::action_score>{{0, 1.f}});
  check_collections_with_float_tolerance(
      examples[0]->pred.decision_scores[1], std::vector<VW::action_score>{{0, 0.5f}, {1, 0.5f}});

  vw.finish_example(examples);
  VW::finish(vw);
  test_base_learner->finish();
  delete test_base_learner;
}
