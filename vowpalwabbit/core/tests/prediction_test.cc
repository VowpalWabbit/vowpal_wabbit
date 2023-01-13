// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Test case validating this issue: https://github.com/VowpalWabbit/vowpal_wabbit/issues/2166
TEST(Predict, PredictModifyingState)
{
  float prediction_one;
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--sgd", "--noconstant", "--learning_rate", "0.1"));
    auto& pre_learn_predict_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& learn_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(*vw, "| 1:1.0");

    vw->predict(pre_learn_predict_example);
    vw->finish_example(pre_learn_predict_example);
    vw->learn(learn_example);
    vw->finish_example(learn_example);
    vw->predict(predict_example);
    prediction_one = predict_example.pred.scalar;
    vw->finish_example(predict_example);
  }

  float prediction_two;
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--sgd", "--noconstant", "--learning_rate", "0.1"));

    auto& learn_example = *VW::read_example(*vw, "0.19574759682114784 | 1:1.430");
    auto& predict_example = *VW::read_example(*vw, "| 1:1.0");

    vw->learn(learn_example);
    vw->finish_example(learn_example);
    vw->predict(predict_example);
    prediction_two = predict_example.pred.scalar;
    vw->finish_example(predict_example);
  }

  EXPECT_FLOAT_EQ(prediction_one, prediction_two);
}
