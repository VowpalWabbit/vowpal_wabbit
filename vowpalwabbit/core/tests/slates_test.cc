// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/slates.h"

#include "gmock/gmock.h"
#include "vw/core/ccb_label.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/multi_ex.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw.h"
#include "vw/test_common/matchers.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

using namespace ::testing;

template <typename LearnFunc, typename PredictFunc>
class test_base
{
public:
  LearnFunc test_learn_func;
  PredictFunc test_predict_func;

  test_base(LearnFunc learn, PredictFunc predict) : test_learn_func(learn), test_predict_func(predict) {}
  static void invoke_learn(test_base<LearnFunc, PredictFunc>& data, VW::multi_ex& examples)
  {
    data.test_learn_func(examples);
  }
  static void invoke_predict(test_base<LearnFunc, PredictFunc>& data, VW::multi_ex& examples)
  {
    data.test_predict_func(examples);
  }
};

template <typename LearnFunc, typename PredictFunc>
std::shared_ptr<VW::LEARNER::learner> make_test_learner(const LearnFunc& learn, const PredictFunc& predict)
{
  auto test_base_data = VW::make_unique<test_base<LearnFunc, PredictFunc>>(learn, predict);
  using func = void (*)(test_base<LearnFunc, PredictFunc>&, VW::multi_ex&);
  auto learn_fptr = &test_base<LearnFunc, PredictFunc>::invoke_learn;
  auto predict_fptr = &test_base<LearnFunc, PredictFunc>::invoke_predict;
  return VW::LEARNER::make_bottom_learner(std::move(test_base_data), static_cast<func>(learn_fptr),
      static_cast<func>(predict_fptr), "mock_reduction", VW::prediction_type_t::DECISION_PROBS, VW::label_type_t::CCB)
      // Set it to something so that the compat VW::finish_example shim is put in place.
      .set_output_example_prediction([](VW::workspace& /* all */, const test_base<LearnFunc, PredictFunc>&,
                                         const VW::multi_ex&, VW::io::logger&) {})
      .build();
}

TEST(Slates, ReductionMockTest)
{
  auto vw = VW::initialize(vwtest::make_args("--slates", "--quiet"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "slates shared 0.8 | ignore_me"));
  examples.push_back(VW::read_example(*vw, "slates action 0 | ignore_me"));
  examples.push_back(VW::read_example(*vw, "slates action 1 | ignore_me"));
  examples.push_back(VW::read_example(*vw, "slates action 1 | ignore_me"));
  examples.push_back(VW::read_example(*vw, "slates slot 0:0.8 | ignore_me"));
  examples.push_back(VW::read_example(*vw, "slates slot 1:0.6 | ignore_me"));

  auto mock_learn_or_pred = [](VW::multi_ex& examples)
  {
    EXPECT_EQ(examples.size(), 6);
    EXPECT_EQ(examples[0]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SHARED);
    EXPECT_EQ(examples[1]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
    EXPECT_EQ(examples[2]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
    EXPECT_EQ(examples[3]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);

    EXPECT_EQ(examples[4]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
    EXPECT_FLOAT_EQ(examples[4]->l.conditional_contextual_bandit.outcome->cost, 0.8f);
    EXPECT_THAT(examples[4]->l.conditional_contextual_bandit.outcome->probabilities,
        Pointwise(ActionScoreEqual(), std::vector<VW::action_score>{{0, 0.8f}}));

    EXPECT_THAT(examples[4]->l.conditional_contextual_bandit.explicit_included_actions, testing::ElementsAre(0));

    EXPECT_EQ(examples[5]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
    EXPECT_FLOAT_EQ(examples[5]->l.conditional_contextual_bandit.outcome->cost, 0.8f);

    EXPECT_THAT(examples[5]->l.conditional_contextual_bandit.outcome->probabilities,
        Pointwise(ActionScoreEqual(), std::vector<VW::action_score>{{2, 0.6f}}));

    EXPECT_THAT(examples[5]->l.conditional_contextual_bandit.explicit_included_actions, testing::ElementsAre(1, 2));

    // Prepare and return the prediction
    VW::v_array<VW::action_score> slot_zero;
    slot_zero.push_back({0, 1.0});
    VW::v_array<VW::action_score> slot_one;
    slot_one.push_back({1, 0.5});
    slot_one.push_back({2, 0.5});
    examples[0]->pred.decision_scores.push_back(slot_zero);
    examples[0]->pred.decision_scores.push_back(slot_one);
  };
  auto test_base_learner = make_test_learner(mock_learn_or_pred, mock_learn_or_pred);
  VW::reductions::slates_data slate_reduction;
  slate_reduction.learn(*test_base_learner, examples);

  // This confirms that the reductions converted the CCB space decision scores back to slates action index space.
  EXPECT_EQ(examples[0]->pred.decision_scores.size(), 2);
  EXPECT_THAT(
      examples[0]->pred.decision_scores[0], Pointwise(ActionScoreEqual(), std::vector<VW::action_score>{{0, 1.f}}));
  EXPECT_THAT(examples[0]->pred.decision_scores[1],
      Pointwise(ActionScoreEqual(), std::vector<VW::action_score>{{0, 0.5f}, {1, 0.5f}}));

  vw->finish_example(examples);
  test_base_learner->finish();
}
