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
