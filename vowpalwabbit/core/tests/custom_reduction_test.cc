// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// this test is a copy from unit_test/prediction_test.cc
// it adds a noop reduction on top

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/reduction_stack.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"
#include "vw/test_common/test_common.h"

#include <memory>

// this file would live in toy_reduction.cc
// toy_reduction.h would define test_reduction_setup(..) fn
// see binary.h and binary.cc for contrast
//
// minimal reduction impl
namespace toy_reduction
{
// global var eek!
// but useful for this minimal test
bool added_to_learner = false;
bool called_learn_predict = false;

// minimal predict/learn fn for test_reduction_setup
template <bool is_learn>
void predict_or_learn(VW::LEARNER::learner& base, VW::example& ec)
{
  EXPECT_TRUE(added_to_learner);
  called_learn_predict = true;

  if (is_learn)
  {
    // this test gets used with that specific label
    EXPECT_NEAR(ec.l.simple.label, 0.195748f, vwtest::EXPLICIT_FLOAT_TOL);
    base.learn(ec);
  }
  else { base.predict(ec); }
}

// minimal setup function for reduction
template <bool test_stack>
std::shared_ptr<VW::LEARNER::learner> test_reduction_setup(VW::setup_base_i& stack_builder)
{
  EXPECT_TRUE(added_to_learner == false);
  EXPECT_TRUE(called_learn_predict == false);

  VW::config::options_i& options = *stack_builder.get_options();

  // do not add when ksvm is present
  // see custom_reduction_builder_check_throw
  if (options.was_supplied("ksvm")) { return nullptr; }

  auto base = stack_builder.setup_base_learner();
  EXPECT_TRUE(base->is_multiline() == false);

  auto ret = VW::LEARNER::make_no_data_reduction_learner(require_singleline(base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(test_reduction_setup<test_stack>))
                 .set_learn_returns_prediction(base->learn_returns_prediction)
                 .build();

  added_to_learner = true;

  if (test_stack)
  {
    std::vector<std::string> enabled_learners;
    ret->get_enabled_learners(enabled_learners);

    // this is the learner stack of instantiated reductions,
    // included itself "test_reduction_name"
    EXPECT_EQ(enabled_learners.size(), 4);
    EXPECT_EQ(enabled_learners[0], "gd");
    EXPECT_EQ(enabled_learners[1], "scorer-identity");
    // this matches string from custom_builder constructor auto-magically [sic]
    EXPECT_EQ(enabled_learners[2], "count_label");
    EXPECT_EQ(enabled_learners[3], "test_reduction_name");
  }
  else { EXPECT_TRUE(false); }

  return ret;
}

void reset_test_state()
{
  added_to_learner = false;
  called_learn_predict = false;
}
}  // namespace toy_reduction
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
class custom_builder : public VW::default_reduction_stack_setup
{
public:
  custom_builder()
  {
    // this is the reduction stack of function pointers
    EXPECT_EQ(std::get<0>(_reduction_stack[0]), "gd");
    EXPECT_EQ(std::get<0>(_reduction_stack[1]), "ksvm");
    EXPECT_EQ(std::get<0>(_reduction_stack[2]), "ftrl");

    EXPECT_GT(_reduction_stack.size(), 77);
    // erase "ksvm" just as a proof of concept
    // see custom_reduction_builder_check_throw below
    _reduction_stack.erase(_reduction_stack.begin() + 1);

    EXPECT_EQ(std::get<0>(_reduction_stack[0]), "gd");
    EXPECT_EQ(std::get<0>(_reduction_stack[1]), "ftrl");

    _reduction_stack.emplace_back("test_reduction_name", toy_reduction::test_reduction_setup<true>);
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
TEST(CustomReduction, General)
{
  toy_reduction::reset_test_state();

  const std::vector<std::string> sgd_args = {"--quiet", "--sgd", "--noconstant", "--learning_rate", "0.1"};
  float prediction_one;
  {
    auto learner_builder = VW::make_unique<custom_builder>();

    EXPECT_TRUE(toy_reduction::added_to_learner == false);
    EXPECT_TRUE(toy_reduction::called_learn_predict == false);
    auto vw = VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(sgd_args), nullptr, nullptr, nullptr,
        nullptr, std::move(learner_builder));
    EXPECT_TRUE(toy_reduction::added_to_learner);
    EXPECT_TRUE(toy_reduction::called_learn_predict == false);

    auto& pre_learn_predict_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& learn_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(*vw, "| 1:1.0");

    EXPECT_TRUE(toy_reduction::called_learn_predict == false);
    vw->predict(pre_learn_predict_example);
    EXPECT_TRUE(toy_reduction::called_learn_predict);

    vw->finish_example(pre_learn_predict_example);
    vw->learn(learn_example);
    vw->finish_example(learn_example);
    vw->predict(predict_example);
    prediction_one = predict_example.pred.scalar;
    vw->finish_example(predict_example);

    EXPECT_TRUE(toy_reduction::added_to_learner);
    EXPECT_TRUE(toy_reduction::called_learn_predict);
  }

  // reset for next part of test
  toy_reduction::reset_test_state();

  float prediction_two;
  {
    EXPECT_TRUE(toy_reduction::added_to_learner == false);
    EXPECT_TRUE(toy_reduction::called_learn_predict == false);
    auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(sgd_args));

    auto& learn_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(*vw, "| 1:1.0");

    vw->learn(learn_example);
    vw->finish_example(learn_example);
    vw->predict(predict_example);
    prediction_two = predict_example.pred.scalar;
    vw->finish_example(predict_example);

    // both should be false since it uses the default stack builder
    EXPECT_TRUE(toy_reduction::added_to_learner == false);
    EXPECT_TRUE(toy_reduction::called_learn_predict == false);
  }

  EXPECT_EQ(prediction_one, prediction_two);
}

TEST(CustomReduction, BuilderCheckThrow)
{
  toy_reduction::reset_test_state();

  const std::vector<std::string> ksvm_args = {"--ksvm", "--quiet"};

  // this test should throw with:
  // Error: unrecognised option '--ksvm'
  // since custom_builder deleted that setup function
  // and that fn registers that cmd option
  {
    auto learner_builder = VW::make_unique<custom_builder>();

    EXPECT_THROW(VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(ksvm_args), nullptr, nullptr,
                     nullptr, nullptr, std::move(learner_builder)),
        VW::vw_exception);

    // check that toy_reduction didn't get added to learner
    EXPECT_TRUE(toy_reduction::added_to_learner == false);
    EXPECT_TRUE(toy_reduction::called_learn_predict == false);
  }

  // this test should not throw with:
  // --ksvm'
  // since its using the default builder that has --ksvm
  {
    EXPECT_NO_THROW(VW::initialize(VW::make_unique<VW::config::options_cli>(ksvm_args)));
  }
}
