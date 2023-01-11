// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"

#include <gtest/gtest.h>

TEST(ConfidenceSequenceRobust, PythonEquivalenceCI)
{
  VW::estimators::confidence_sequence_robust csr;

  for (size_t i = 0; i < 200; ++i)
  {
    csr.update(1.1, 1);
    csr.update(1.1, 0);
  }

  double lb = csr.lower_bound();
  double ub = csr.upper_bound();

  // Compare to test_confidence_sequence_robust.py
  EXPECT_FLOAT_EQ(lb, 0.4574146652500113);
  EXPECT_FLOAT_EQ(ub, 0.5423597665906932);
}

#if !defined(__APPLE__) && !defined(_WIN32)
TEST(ConfidenceSequenceRobust, PythonEquivalenceBrentq)
{
  // Set up state
  VW::estimators::confidence_sequence_robust csr(0.2 / 16.0);
  csr.lower.t = 88;
  csr.lower.gt.t = 88;
  double s = 139.8326745448;
  double thres = 3.6888794541139363;
  std::map<uint64_t, double> memo;
  memo[0] = 39.016179559463588;
  memo[1] = 20.509121588503511;
  memo[2] = 10.821991705197142;
  double min_mu = 0.0;
  double max_mu = 1.0;

  // Get root
  double root = csr.lower.root_brentq(s, thres, memo, min_mu, max_mu);

  // Compare root to test_confidence_sequence_robust.py
  EXPECT_NEAR(root, 0.8775070821950665, 1e-6);

  // Test that root of objective function is 0
  auto test_root = csr.lower.log_wealth_mix(root, s, thres, memo) - thres;
  EXPECT_NEAR(test_root, 0.0, 1e-6);
}
#endif
