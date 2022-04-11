// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

// this test is a copy from unit_test/prediction_test.cc
// it adds a noop reduction on top

#include "reduction_stack.h"
#include "vw.h"
#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw_fwd.h"

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
void predict_or_learn(char&, VW::LEARNER::single_learner& base, VW::example& ec)
{
  BOOST_CHECK(added_to_learner);
  called_learn_predict = true;

  if (is_learn)
  {
    // this test gets used with that specific label
    BOOST_CHECK_CLOSE(ec.l.simple.label, 0.195748, 1E-3);
    base.learn(ec);
  }
  else
  {
    base.predict(ec);
  }
}

// minimal setup function for reduction
template <bool test_stack>
VW::LEARNER::base_learner* test_reduction_setup(VW::setup_base_i& stack_builder)
{
  BOOST_CHECK(added_to_learner == false);
  BOOST_CHECK(called_learn_predict == false);

  VW::config::options_i& options = *stack_builder.get_options();

  // do not add when ksvm is present
  // see custom_reduction_builder_check_throw
  if (options.was_supplied("ksvm")) { return nullptr; }

  auto base = stack_builder.setup_base_learner();
  BOOST_CHECK(base->is_multiline() == false);

  auto ret = VW::LEARNER::make_no_data_reduction_learner(as_singleline(base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(test_reduction_setup<test_stack>))
                 .set_learn_returns_prediction(base->learn_returns_prediction)
                 .build();

  added_to_learner = true;

  if (test_stack)
  {
    std::vector<std::string> enabled_reductions;
    ret->get_enabled_reductions(enabled_reductions);

    // this is the learner stack of instantiated reductions,
    // included itself "test_reduction_name"
    BOOST_CHECK_EQUAL(enabled_reductions.size(), 4);
    BOOST_CHECK_EQUAL(enabled_reductions[0], "gd");
    BOOST_CHECK_EQUAL(enabled_reductions[1], "scorer-identity");
    // this matches string from custom_builder constructor auto-magically [sic]
    BOOST_CHECK_EQUAL(enabled_reductions[2], "count_label");
    BOOST_CHECK_EQUAL(enabled_reductions[3], "test_reduction_name");
  }
  else
  {
    BOOST_CHECK(false);
  }

  return make_base(*ret);
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
struct custom_builder : VW::default_reduction_stack_setup
{
  custom_builder()
  {
    // this is the reduction stack of function pointers
    BOOST_CHECK_EQUAL(std::get<0>(reduction_stack[0]), "gd");
    BOOST_CHECK_EQUAL(std::get<0>(reduction_stack[1]), "ksvm");
    BOOST_CHECK_EQUAL(std::get<0>(reduction_stack[2]), "ftrl");

    BOOST_CHECK_GT(reduction_stack.size(), 77);
    // erase "ksvm" just as a proof of concept
    // see custom_reduction_builder_check_throw below
    reduction_stack.erase(reduction_stack.begin() + 1);

    BOOST_CHECK_EQUAL(std::get<0>(reduction_stack[0]), "gd");
    BOOST_CHECK_EQUAL(std::get<0>(reduction_stack[1]), "ftrl");

    reduction_stack.emplace_back("test_reduction_name", toy_reduction::test_reduction_setup<true>);
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
BOOST_AUTO_TEST_CASE(custom_reduction_test)
{
  toy_reduction::reset_test_state();

  std::string sgd_args = "--quiet --sgd --noconstant --learning_rate 0.1";

  float prediction_one;
  {
    auto learner_builder = VW::make_unique<custom_builder>();

    BOOST_CHECK(toy_reduction::added_to_learner == false);
    BOOST_CHECK(toy_reduction::called_learn_predict == false);
    auto& vw = *VW::initialize_with_builder(sgd_args, nullptr, false, nullptr, nullptr, std::move(learner_builder));
    BOOST_CHECK(toy_reduction::added_to_learner);
    BOOST_CHECK(toy_reduction::called_learn_predict == false);

    auto& pre_learn_predict_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& learn_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(vw, "| 1:1.0");

    BOOST_CHECK(toy_reduction::called_learn_predict == false);
    vw.predict(pre_learn_predict_example);
    BOOST_CHECK(toy_reduction::called_learn_predict);

    vw.finish_example(pre_learn_predict_example);
    vw.learn(learn_example);
    vw.finish_example(learn_example);
    vw.predict(predict_example);
    prediction_one = predict_example.pred.scalar;
    vw.finish_example(predict_example);
    VW::finish(vw);

    BOOST_CHECK(toy_reduction::added_to_learner);
    BOOST_CHECK(toy_reduction::called_learn_predict);
  }

  // reset for next part of test
  toy_reduction::reset_test_state();

  float prediction_two;
  {
    BOOST_CHECK(toy_reduction::added_to_learner == false);
    BOOST_CHECK(toy_reduction::called_learn_predict == false);
    auto& vw = *VW::initialize(sgd_args);

    auto& learn_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(vw, "| 1:1.0");

    vw.learn(learn_example);
    vw.finish_example(learn_example);
    vw.predict(predict_example);
    prediction_two = predict_example.pred.scalar;
    vw.finish_example(predict_example);
    VW::finish(vw);

    // both should be false since it uses the default stack builder
    BOOST_CHECK(toy_reduction::added_to_learner == false);
    BOOST_CHECK(toy_reduction::called_learn_predict == false);
  }

  BOOST_CHECK_EQUAL(prediction_one, prediction_two);
}

BOOST_AUTO_TEST_CASE(custom_reduction_builder_check_throw)
{
  toy_reduction::reset_test_state();

  std::string ksvm_args = "--ksvm --quiet";

  // this test should throw with:
  // Error: unrecognised option '--ksvm'
  // since custom_builder deleted that setup function
  // and that fn registers that cmd option
  {
    auto learner_builder = VW::make_unique<custom_builder>();

    BOOST_CHECK_THROW(
        VW::initialize_with_builder(ksvm_args, nullptr, false, nullptr, nullptr, std::move(learner_builder)),
        VW::vw_exception);

    // check that toy_reduction didn't get added to learner
    BOOST_CHECK(toy_reduction::added_to_learner == false);
    BOOST_CHECK(toy_reduction::called_learn_predict == false);
  }

  // this test should not throw with:
  // --ksvm'
  // since its using the default builder that has --ksvm
  {
    BOOST_CHECK_NO_THROW(VW::finish(*VW::initialize(ksvm_args)));
  }
}
