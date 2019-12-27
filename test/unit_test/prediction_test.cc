#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "vw.h"

// Test case validating this issue: https://github.com/VowpalWabbit/vowpal_wabbit/issues/2166
BOOST_AUTO_TEST_CASE(predict_modifying_state)
{
  float prediction_one;
  {
    auto& vw = *VW::initialize("--quiet --sgd --noconstant --learning_rate 0.1");
    auto& pre_learn_predict_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& learn_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(vw, "| 1:1.0");

    vw.predict(pre_learn_predict_example);
    vw.finish_example(pre_learn_predict_example);
    vw.learn(learn_example);
    vw.finish_example(learn_example);
    vw.predict(predict_example);
    prediction_one = predict_example.pred.scalar;
    vw.finish_example(predict_example);
    VW::finish(vw);
  }

  float prediction_two;
  {
    auto& vw = *VW::initialize("--quiet --sgd --noconstant --learning_rate 0.1");

    auto& learn_example = *VW::read_example(vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(vw, "| 1:1.0");

    vw.learn(learn_example);
    vw.finish_example(learn_example);
    vw.predict(predict_example);
    prediction_two = predict_example.pred.scalar;
    vw.finish_example(predict_example);
    VW::finish(vw);
  }

  BOOST_CHECK_EQUAL(prediction_one, prediction_two);
}
