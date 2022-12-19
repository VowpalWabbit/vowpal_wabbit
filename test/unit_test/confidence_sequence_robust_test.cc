// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"

#include "test_common.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

constexpr double CSR_DOUBLE_TOL = 0.000001;

BOOST_AUTO_TEST_CASE(confidence_sequence_robust_test)
{
  VW::confidence_sequence_robust csr;

  for (size_t i = 0; i < 200; ++i)
  {
    csr.update(1.1, 1);
    csr.update(1.1, 0);
  }

  double lb = csr.lower_bound();
  double ub = csr.upper_bound();

  // Compare to confidence_sequence_robust_test.py
  BOOST_CHECK_CLOSE(lb, 0.4574146652500113, CSR_DOUBLE_TOL);
  BOOST_CHECK_CLOSE(ub, 0.5423597665906932, CSR_DOUBLE_TOL);

  // Test brentq minimizer separately
  double s = 219.99999999999926;
  double thres = 3.6888794541139363;
  std::map<uint64_t, double> memo;
  memo[0] = 12.680412758057498;
  memo[1] = 4.354160995282192;
  double min_mu = 0.0;
  double max_mu = 1.0;

  double root = csr.upper.root_brentq(s, thres, memo, min_mu, max_mu);

  // Compare to confidence_sequence_robust_test.py
  BOOST_CHECK_CLOSE(root, 0.4576402334093068, CSR_DOUBLE_TOL);
}
