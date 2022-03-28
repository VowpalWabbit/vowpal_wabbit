// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "loss_functions.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

#include "named_labels.h"
#include "test_common.h"

BOOST_AUTO_TEST_CASE(squared_loss_test)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("squared");

  auto loss = getLossFunction(vw, loss_type);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(0.0f, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.01f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.01812692f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.02f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.04f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.2f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(2.0f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_label_is_greater_than_prediction_test1)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.4f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.006f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.011307956f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.012f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.0144f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.12f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(1.2f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_label_is_greater_than_prediction_test2)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.25f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.74f;
  constexpr float prediction = 0.18f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.2352f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0780035332f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.084f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.7056f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.84f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(1.5f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_label_is_greater_than_prediction_test3)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.2f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.84f;
  constexpr float prediction = 0.48f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.10368f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.05322823597f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0576f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.331776f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.576f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(1.6f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_label_is_greater_than_prediction_test4)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.3f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.29f;
  constexpr float prediction = 0.06f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.03703f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.03004760586f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0322f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.103684f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.322f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(1.4f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_label_is_greater_than_prediction_test5)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.25f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.50f;
  constexpr float prediction = 0.09f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.126075f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.05710972967f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0615f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.378225f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.615f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(1.5f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_prediction_is_greater_than_label_test1)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.4f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.4f;
  constexpr float prediction = 0.5f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.004f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.007688365f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.008f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.0064f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.08f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.8f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_prediction_is_greater_than_label_test2)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.25f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.57f;
  constexpr float prediction = 0.64f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.001225f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.003413940285f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.0035f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.001225f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.035f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.5f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_prediction_is_greater_than_label_test3)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.2f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.79f;
  constexpr float prediction = 0.91f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.00288f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.004705267302f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.0048f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.002304f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.048f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.4f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_prediction_is_greater_than_label_test4)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.2f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.02f;
  constexpr float prediction = 0.29f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.01458f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.01058685143f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.0108f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.011664f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.108f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.4f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_prediction_is_greater_than_label_test5)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.4f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.24f;
  constexpr float prediction = 0.44f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.016f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.01537673072f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.016f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.0256f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.16f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.8f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}
BOOST_AUTO_TEST_CASE(expectile_loss_parameter_equals_zero_test)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(0.0f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.01f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.01812692f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.02f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.04f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(-0.2f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(2.0f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(expectile_loss_parameter_equals_one_test)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("expectile");
  constexpr float parameter(1.0f);

  auto loss = getLossFunction(vw, loss_type, parameter);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_EQUAL(loss_type, loss->getType());
  BOOST_CHECK_CLOSE(parameter, loss->getParameter(), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.0f, loss->getLoss(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0f, loss->getUpdate(prediction, label, update_scale, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0f, loss->getUnsafeUpdate(prediction, label, update_scale), FLOAT_TOL);

  BOOST_CHECK_CLOSE(0.0f, loss->getSquareGrad(prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0f, loss->first_derivative(&sd, prediction, label), FLOAT_TOL);
  BOOST_CHECK_CLOSE(0.0f, loss->second_derivative(&sd, prediction, label), FLOAT_TOL);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(compare_expectile_loss_with_squared_loss_test)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type_expectile("expectile");
  const std::string loss_type_squared("squared");
  constexpr float parameter(0.3f);

  auto loss_expectile = getLossFunction(vw, loss_type_expectile, parameter);
  auto loss_squared = getLossFunction(vw, loss_type_squared);
  shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;
  constexpr float eta = 0.1f;     // learning rate
  constexpr float weight = 1.0f;  // example weight

  constexpr float label = 0.5f;
  constexpr float prediction = 0.4f;
  constexpr float update_scale = eta * weight;
  constexpr float pred_per_update = 1.0f;  // Use dummy value here, see gd.cc for details.

  BOOST_CHECK_CLOSE(loss_expectile->getLoss(&sd, prediction, label),
      loss_squared->getLoss(&sd, prediction, label) * parameter, FLOAT_TOL);
  BOOST_CHECK_CLOSE(loss_expectile->getUpdate(prediction, label, update_scale, pred_per_update),
      loss_squared->getUpdate(prediction, label, update_scale * parameter, pred_per_update), FLOAT_TOL);
  BOOST_CHECK_CLOSE(loss_expectile->getUnsafeUpdate(prediction, label, update_scale),
      loss_squared->getUnsafeUpdate(prediction, label, update_scale * parameter), FLOAT_TOL);

  BOOST_CHECK_CLOSE(loss_expectile->getSquareGrad(prediction, label),
      loss_squared->getSquareGrad(prediction, label) * parameter * parameter, FLOAT_TOL);
  BOOST_CHECK_CLOSE(loss_expectile->first_derivative(&sd, prediction, label),
      loss_squared->first_derivative(&sd, prediction, label) * parameter, FLOAT_TOL);
  BOOST_CHECK_CLOSE(loss_expectile->second_derivative(&sd, prediction, label),
      loss_squared->second_derivative(&sd, prediction, label) * parameter, FLOAT_TOL);

  VW::finish(vw);
}
