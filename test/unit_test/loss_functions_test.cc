// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "loss_functions.h"
#include "test_common.h"

BOOST_AUTO_TEST_CASE(squared_loss_test)
{
  auto& vw = *VW::initialize("--quiet");
  const std::string loss_type("squared");

  auto loss = getLossFunction(vw, loss_type);

  BOOST_CHECK_EQUAL(loss_type, loss->getType());

  VW::finish(vw);
}
