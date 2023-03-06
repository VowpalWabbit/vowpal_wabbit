// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// this test is a copy from unit_test/prediction_test.cc
// it adds a noop reduction on top

#include "vw/config/options_cli.h"
#include "vw/core/learner.h"
#include "vw/core/reduction_stack.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"

// this file would live in minimal_reduction.cc
// minimal_reduction.h would define test_reduction_setup(..) fn
// see binary.h and binary.cc for contrast
//
// minimal reduction impl
namespace minimal_reduction
{
// global var eek!
// but useful for this minimal test
bool added_to_learner = false;
bool called_learn_predict = false;

// minimal predict/learn fn for test_reduction_setup
template <bool is_learn>
void predict_or_learn(VW::LEARNER::learner& base, VW::example& ec)
{
  called_learn_predict = true;

  if (is_learn) { base.learn(ec); }
  else { base.predict(ec); }
}

// minimal setup function for reduction
std::shared_ptr<VW::LEARNER::learner> test_reduction_setup(VW::setup_base_i& stack_builder)
{
  EXPECT_TRUE(added_to_learner == false);
  EXPECT_TRUE(called_learn_predict == false);

  auto base = stack_builder.setup_base_learner();
  EXPECT_TRUE(base->is_multiline() == false);

  auto ret = VW::LEARNER::make_no_data_reduction_learner(require_singleline(base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(test_reduction_setup))
                 .set_learn_returns_prediction(base->learn_returns_prediction)
                 .build();

  added_to_learner = true;

  return ret;
}

void reset_test_state()
{
  added_to_learner = false;
  called_learn_predict = false;
}
}  // namespace minimal_reduction
//
//
//
//

// minimal builder that pushes test_reduction_setup fn to the
// default stack (reduction_stack)
// inherits from the default behaviour stack builder
//
// custom_builder can be augmented to do heavier edits (reorder/remove)
// on reduction_stack
class custom_simple_builder : public VW::default_reduction_stack_setup
{
public:
  custom_simple_builder()
  {
    EXPECT_GT(_reduction_stack.size(), 77);
    _reduction_stack.emplace_back("test_reduction_name", minimal_reduction::test_reduction_setup);
  }
};

// This test will push a new reduction defined in this same file.
// This reduction doesn't do anything but set a bool to true
// and assert on the enabled reductions.
// This test approximates one way users can use VW library mode
// and customize the behaviour of the stack.
//
// We can also use this as an example to craft more specific tests
// at specific parts of the stack i.e. push a test reduction that
// asserts some state right on top of cb_adf.
//
// these test reductions are useful for CI and advanced testing but are not
// important enough to pollute setup_base(..) default stack
TEST(MinimalReduction, MinimalReductionTest)
{
  minimal_reduction::reset_test_state();

  const std::vector<std::string> sgd_args = {"--quiet", "--sgd", "--noconstant", "--learning_rate", "0.1"};

  float prediction_one;
  {
    auto learner_builder = VW::make_unique<custom_simple_builder>();

    EXPECT_TRUE(minimal_reduction::added_to_learner == false);
    EXPECT_TRUE(minimal_reduction::called_learn_predict == false);
    auto vw = VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(sgd_args), nullptr, nullptr, nullptr,
        nullptr, std::move(learner_builder));
    EXPECT_TRUE(minimal_reduction::added_to_learner);
    EXPECT_TRUE(minimal_reduction::called_learn_predict == false);

    auto& pre_learn_predict_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& learn_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(*vw, "| 1:1.0");

    EXPECT_TRUE(minimal_reduction::called_learn_predict == false);
    vw->predict(pre_learn_predict_example);
    EXPECT_TRUE(minimal_reduction::called_learn_predict);

    vw->finish_example(pre_learn_predict_example);
    vw->learn(learn_example);
    vw->finish_example(learn_example);
    vw->predict(predict_example);
    prediction_one = predict_example.pred.scalar;
    vw->finish_example(predict_example);

    EXPECT_TRUE(minimal_reduction::added_to_learner);
    EXPECT_TRUE(minimal_reduction::called_learn_predict);
  }

  // reset for next part of test
  minimal_reduction::reset_test_state();

  float prediction_two;
  {
    EXPECT_TRUE(minimal_reduction::added_to_learner == false);
    EXPECT_TRUE(minimal_reduction::called_learn_predict == false);
    auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(sgd_args));

    auto& learn_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(*vw, "| 1:1.0");

    vw->learn(learn_example);
    vw->finish_example(learn_example);
    vw->predict(predict_example);
    prediction_two = predict_example.pred.scalar;
    vw->finish_example(predict_example);

    // both should be false since it uses the default stack builder
    EXPECT_TRUE(minimal_reduction::added_to_learner == false);
    EXPECT_TRUE(minimal_reduction::called_learn_predict == false);
  }

  EXPECT_EQ(prediction_one, prediction_two);
}
