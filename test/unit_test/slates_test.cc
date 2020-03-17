#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "vw.h"
#include "example.h"

#include <vector>
#include "slates.h"
#include "slates_label.h"
#include "ccb_label.h"
#include "learner.h"

template <typename LearnFunc, typename PredictFunc>
struct test_base
{
  LearnFunc test_learn_func;
  PredictFunc test_predict_func;

  test_base(LearnFunc learn, PredictFunc predict) : test_learn_func(learn), test_predict_func(predict) {}
  static void invoke_learn(test_base<LearnFunc, PredictFunc>& data, LEARNER::multi_learner& base, multi_ex& examples)
  {
      data.test_learn_func(examples);
  }
  static void invoke_predict(test_base<LearnFunc, PredictFunc>& data, LEARNER::multi_learner& base, multi_ex& examples)
  {
      data.test_predict_func(examples);
  }
};


template <typename LearnFunc, typename PredictFunc>
LEARNER::base_learner* make_test_learner(LearnFunc learn, PredictFunc predict)
{
  auto test_base_data = scoped_calloc_or_throw<test_base<LearnFunc, PredictFunc>>(learn, predict);
  using func = void (*)(test_base<LearnFunc, PredictFunc>&, LEARNER::multi_learner&, multi_ex&);
  auto learn_fptr = &test_base<LearnFunc, PredictFunc>::invoke_learn;
  auto predict_fptr = &test_base<LearnFunc, PredictFunc>::invoke_predict;
  auto& l = LEARNER::init_learner(test_base_data, (LEARNER::multi_learner*)nullptr, static_cast<func>(learn_fptr), static_cast<func>(predict_fptr));
  return LEARNER::make_base(l);
}

BOOST_AUTO_TEST_CASE(slates_reduction_mock_test)
{
  auto& vw = *VW::initialize("--slates --quiet");
  multi_ex examples;
  examples.push_back(VW::read_example(vw, std::string("slates shared 0.8 | ignore_me")));
  examples.push_back(VW::read_example(vw, std::string("slates action 0 | ignore_me")));
  examples.push_back(VW::read_example(vw, std::string("slates action 0 | ignore_me")));
  examples.push_back(VW::read_example(vw, std::string("slates action 0 | ignore_me")));
  examples.push_back(VW::read_example(vw, std::string("slates action 1 | ignore_me")));
  examples.push_back(VW::read_example(vw, std::string("slates action 1 | ignore_me")));
  examples.push_back(VW::read_example(vw, std::string("slates slot 1:0.8 | ignore_me")));
  examples.push_back(VW::read_example(vw, std::string("slates slot 0:0.6 | ignore_me")));

  vw.predict(examples);

  auto learn_assert = [](multi_ex& examples){};
  auto pred_assert = [](multi_ex& examples){};
  auto test_base_learner = make_test_learner(learn_assert, pred_assert);

  // auto& decision_scores = examples[0]->pred.decision_scores;
  // BOOST_CHECK_EQUAL(decision_scores.size(), 3);

  // BOOST_CHECK_EQUAL(decision_scores[0].size(), 1);
  // BOOST_CHECK_EQUAL(decision_scores[0][0].action, 0);
  // BOOST_CHECK_CLOSE(decision_scores[0][0].score, 1.f, FLOAT_TOL);

  // BOOST_CHECK_EQUAL(decision_scores[1].size(), 1);
  // BOOST_CHECK_EQUAL(decision_scores[1][0].action, 3);
  // BOOST_CHECK_CLOSE(decision_scores[1][0].score, 1.f, FLOAT_TOL);

  // BOOST_CHECK_EQUAL(decision_scores[2].size(), 1);
  // BOOST_CHECK_EQUAL(decision_scores[2][0].action, 1);
  // BOOST_CHECK_CLOSE(decision_scores[2][0].score, 1.f, FLOAT_TOL);

  vw.finish_example(examples);
  VW::finish(vw);
}
