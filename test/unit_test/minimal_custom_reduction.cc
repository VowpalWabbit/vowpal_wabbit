#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

// this test is a copy from unit_test/prediction_test.cc
// it adds a noop reduction on top

#include "vw.h"
#include "reductions_fwd.h"
#include "reduction_stack.h"

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
void predict_or_learn(char&, VW::LEARNER::single_learner& base, example& ec)
{
  called_learn_predict = true;

  if (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }
}

// minimal setup function for reduction
VW::LEARNER::base_learner* test_reduction_setup(VW::setup_base_i& stack_builder)
{
  BOOST_CHECK(added_to_learner == false);
  BOOST_CHECK(called_learn_predict == false);

  auto base = stack_builder.setup_base_learner();
  BOOST_CHECK(base->is_multiline == false);

  auto ret = VW::LEARNER::make_no_data_reduction_learner(as_singleline(base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(test_reduction_setup))
                 .set_learn_returns_prediction(base->learn_returns_prediction)
                 .build();

  added_to_learner = true;

  return make_base(*ret);
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
struct custom_simple_builder : VW::default_reduction_stack_setup
{
  custom_simple_builder()
  {
    BOOST_CHECK_GT(reduction_stack.size(), 77);
    reduction_stack.emplace_back("test_reduction_name", minimal_reduction::test_reduction_setup);
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
BOOST_AUTO_TEST_CASE(minimal_reduction_test)
{
  minimal_reduction::reset_test_state();

  std::string sgd_args = "--quiet --sgd --noconstant --learning_rate 0.1";

  float prediction_one;
  {
    auto learner_builder = VW::make_unique<custom_simple_builder>();

    BOOST_CHECK(minimal_reduction::added_to_learner == false);
    BOOST_CHECK(minimal_reduction::called_learn_predict == false);
    auto& vw = *VW::initialize_with_builder(sgd_args, nullptr, false, nullptr, nullptr, std::move(learner_builder));
    BOOST_CHECK(minimal_reduction::added_to_learner);
    BOOST_CHECK(minimal_reduction::called_learn_predict == false);

    auto& pre_learn_predict_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& learn_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(vw, "| 1:1.0");

    BOOST_CHECK(minimal_reduction::called_learn_predict == false);
    vw.predict(pre_learn_predict_example);
    BOOST_CHECK(minimal_reduction::called_learn_predict);

    vw.finish_example(pre_learn_predict_example);
    vw.learn(learn_example);
    vw.finish_example(learn_example);
    vw.predict(predict_example);
    prediction_one = predict_example.pred.scalar;
    vw.finish_example(predict_example);
    VW::finish(vw);

    BOOST_CHECK(minimal_reduction::added_to_learner);
    BOOST_CHECK(minimal_reduction::called_learn_predict);
  }

  // reset for next part of test
  minimal_reduction::reset_test_state();

  float prediction_two;
  {
    BOOST_CHECK(minimal_reduction::added_to_learner == false);
    BOOST_CHECK(minimal_reduction::called_learn_predict == false);
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
    BOOST_CHECK(minimal_reduction::added_to_learner == false);
    BOOST_CHECK(minimal_reduction::called_learn_predict == false);
  }

  BOOST_CHECK_EQUAL(prediction_one, prediction_two);
}
