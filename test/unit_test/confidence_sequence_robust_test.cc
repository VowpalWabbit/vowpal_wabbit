// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"

#include "test_common.h"

#include <boost/math/tools/minima.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(confidence_sequence_robust_test)
{
  VW::confidence_sequence_robust csr;
  for (int i = 0; i < 200; ++i)
  {
    csr.update(1.1, 1);
    csr.update(1.1, 0);
  }
  std::cout << csr.lower_bound() << " " << csr.upper_bound() << "\n";
}
