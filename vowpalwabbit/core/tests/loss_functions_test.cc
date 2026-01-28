// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/loss_functions.h"

#include "vw/core/named_labels.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(LossFunctions, SquaredLossTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("squared");

  auto loss = get_loss_function(*vw, loss_type);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(0.0f, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.01f, loss->get_loss(&sd, prediction, label));
  EXPECT_NEAR(0.01812692f, loss->get_update(prediction, label, update_scale, pred_per_update), 1e-6f);
  EXPECT_FLOAT_EQ(0.02f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.04f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.2f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(2.0f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossLabelIsGreaterThanPredictionTest1)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.4f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.006f, loss->get_loss(&sd, prediction, label));
  EXPECT_NEAR(0.011307956f, loss->get_update(prediction, label, update_scale, pred_per_update), 1e-6f);
  EXPECT_FLOAT_EQ(0.012f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.0144f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.12f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(1.2f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossParameterEqualsZeroTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.0f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.01f, loss->get_loss(&sd, prediction, label));
  EXPECT_NEAR(0.01812692f, loss->get_update(prediction, label, update_scale, pred_per_update), 1e-6f);
  EXPECT_FLOAT_EQ(0.02f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.04f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.2f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(2.0f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossParameterEqualsOneTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(1.0f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.0f, loss->get_loss(&sd, prediction, label));
  EXPECT_NEAR(0.0f, loss->get_update(prediction, label, update_scale, pred_per_update), 1e-6f);
  EXPECT_FLOAT_EQ(0.0f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.0f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(0.0f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.0f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, CompareExpectileLossWithSquaredLossTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type_expectile("expectile");
  const std::string loss_type_squared("squared");
  constexpr float parameter(0.3f);

  auto loss_expectile = get_loss_function(*vw, loss_type_expectile, parameter);
  auto loss_squared = get_loss_function(*vw, loss_type_squared);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.4f;
  constexpr float prediction = 0.5f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_FLOAT_EQ(
      loss_expectile->get_loss(&sd, prediction, label), loss_squared->get_loss(&sd, prediction, label) * parameter);
  EXPECT_FLOAT_EQ(loss_expectile->get_update(prediction, label, update_scale, pred_per_update),
      loss_squared->get_update(prediction, label, update_scale * parameter, pred_per_update));
  EXPECT_FLOAT_EQ(loss_expectile->get_unsafe_update(prediction, label, update_scale),
      loss_squared->get_unsafe_update(prediction, label, update_scale * parameter));

  EXPECT_FLOAT_EQ(loss_expectile->get_square_grad(prediction, label),
      loss_squared->get_square_grad(prediction, label) * parameter * parameter);
  EXPECT_FLOAT_EQ(loss_expectile->first_derivative(&sd, prediction, label),
      loss_squared->first_derivative(&sd, prediction, label) * parameter);
  EXPECT_FLOAT_EQ(loss_expectile->second_derivative(&sd, prediction, label),
      loss_squared->second_derivative(&sd, prediction, label) * parameter);
}
