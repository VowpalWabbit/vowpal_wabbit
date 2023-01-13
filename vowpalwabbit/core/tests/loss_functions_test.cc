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
  EXPECT_FLOAT_EQ(0.01812692f, loss->get_update(prediction, label, update_scale, pred_per_update));
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
  EXPECT_FLOAT_EQ(0.011307956f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(0.012f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.0144f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.12f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(1.2f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossLabelIsGreaterThanPredictionTest2)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.25f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.74f;
  constexpr float prediction = 0.18f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.2352f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.0780035332f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(0.084f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.7056f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.84f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(1.5f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossLabelIsGreaterThanPredictionTest3)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.2f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.84f;
  constexpr float prediction = 0.48f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.10368f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.05322823597f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(0.0576f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.331776f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.576f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(1.6f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossLabelIsGreaterThanPredictionTest4)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.3f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.29f;
  constexpr float prediction = 0.06f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.03703f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.03004760586f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(0.0322f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.103684f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.322f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(1.4f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossLabelIsGreaterThanPredictionTest5)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.25f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.50f;
  constexpr float prediction = 0.09f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.126075f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.05710972967f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(0.0615f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.378225f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(-0.615f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(1.5f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossPredictionIsGreaterThanLabelTest1)
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

  constexpr float label = 0.4f;
  constexpr float prediction = 0.5f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.004f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(-0.007688365f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(-0.008f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.0064f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(0.08f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.8f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossPredictionIsGreaterThanLabelTest2)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.25f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.57f;
  constexpr float prediction = 0.64f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.001225f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(-0.003413938f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(-0.0035f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.001225f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(0.035f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.5f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossPredictionIsGreaterThanLabelTest3)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.2f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.79f;
  constexpr float prediction = 0.91f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.00288f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(-0.004705267302f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(-0.0048f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.002304f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(0.048f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.4f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossPredictionIsGreaterThanLabelTest4)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  const std::string loss_type("expectile");
  constexpr float parameter(0.2f);

  auto loss = get_loss_function(*vw, loss_type, parameter);
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.02f;
  constexpr float prediction = 0.29f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.01458f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(-0.01058685143f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(-0.0108f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.011664f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(0.108f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.4f, loss->second_derivative(&sd, prediction, label));
}

TEST(LossFunctions, ExpectileLossPredictionIsGreaterThanLabelTest5)
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

  constexpr float label = 0.24f;
  constexpr float prediction = 0.44f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  EXPECT_EQ(loss_type, loss->get_type());
  EXPECT_FLOAT_EQ(parameter, loss->get_parameter());

  EXPECT_FLOAT_EQ(0.016f, loss->get_loss(&sd, prediction, label));
  EXPECT_FLOAT_EQ(-0.01537673072f, loss->get_update(prediction, label, update_scale, pred_per_update));
  EXPECT_FLOAT_EQ(-0.016f, loss->get_unsafe_update(prediction, label, update_scale));

  EXPECT_FLOAT_EQ(0.0256f, loss->get_square_grad(prediction, label));
  EXPECT_FLOAT_EQ(0.16f, loss->first_derivative(&sd, prediction, label));
  EXPECT_FLOAT_EQ(0.8f, loss->second_derivative(&sd, prediction, label));
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
  EXPECT_FLOAT_EQ(0.01812692f, loss->get_update(prediction, label, update_scale, pred_per_update));
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
  EXPECT_FLOAT_EQ(0.0f, loss->get_update(prediction, label, update_scale, pred_per_update));
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
