// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/estimators/confidence_sequence_robust.h"

#include <gtest/gtest.h>

TEST(ConfidenceSequenceRobust, PythonEquivalenceCI)
{
  VW::estimators::confidence_sequence_robust csr(1e-6, false, .05 / 16.0);
  csr.update(0.265260433, 0.4);
  csr.update(0.210534972, 0);
  csr.update(0.183917071, 1);
  csr.update(0.559713528, 1);
  csr.update(0.595640365, 1);
  csr.update(7.180584551, 0.9);
  csr.update(0.138659127, 0.9);
  csr.update(0.663108867, 0);
  csr.update(0.678420014, 0.1);
  csr.update(0.123112832, 0.9);
  csr.update(0.703156298, 1);
  csr.update(0.713376195, 0.1);
  csr.update(0.722519043, 1);
  csr.update(0.730768415, 0.6);
  csr.update(0.107544107, 1);
  csr.update(0.745132612, 1);
  csr.update(0.751442272, 0.9);
  csr.update(4.986560376, 0.4);
  csr.update(0.099391065, 1);
  csr.update(37.85031381, 0.9);
  csr.update(0.096127937, 0.9);
  csr.update(0.094648809, 0.6);
  csr.update(0.78106104, 0.9);
  csr.update(0.784993688, 0.9);
  csr.update(0.090697586, 0.6);
  csr.update(0.792238169, 0.4);
  csr.update(0.088398536, 1);
  csr.update(0.798782221, 0.6);
  csr.update(0.801821345, 0.3);
  csr.update(0.804722505, 0.9);
  csr.update(0.807496314, 0.9);
  csr.update(0.810154443, 0.1);
  csr.update(0.082674622, 1);
  csr.update(0.815150197, 0.6);
  csr.update(0.081068034, 0.4);
  csr.update(0.080309513, 0.1);
  csr.update(0.079578481, 0.4);
  csr.update(0.078873344, 1);
  csr.update(0.82609249, 1);
  csr.update(0.828055494, 0.7);
  csr.update(0.076898241, 1);
  csr.update(0.076283066, 1);
  csr.update(0.075686285, 1);
  csr.update(0.075107653, 0.4);
  csr.update(0.074546428, 1);
  csr.update(0.074001442, 1);
  csr.update(0.073472059, 0.9);
  csr.update(0.072958202, 0.9);
  csr.update(0.072457789, 1);
  csr.update(0.844657383, 0.9);
  csr.update(0.071496447, 0.9);
  csr.update(0.071034351, 1);
  csr.update(0.070584037, 0.9);
  csr.update(0.070145612, 0.6);
  csr.update(0.069717158, 1);
  csr.update(0.069298895, 0.5);
  csr.update(0.853830002, 0.3);
  csr.update(0.855015059, 1);
  csr.update(0.068101742, 1);
  csr.update(0.067721242, 0.9);
  csr.update(0.067348406, 0.1);
  csr.update(0.066983637, 0.3);
  csr.update(0.860565217, 0);
  csr.update(0.066277134, 0.6);
  csr.update(42.83659821, 0.4);
  csr.update(0.863622193, 0.3);
  csr.update(0.864597097, 1);
  csr.update(0.064949207, 0.1);
  csr.update(0.064633158, 0.6);
  csr.update(0.867408224, 0.9);
  csr.update(0.868308821, 0.9);
  csr.update(0.063721357, 0.9);
  csr.update(0.870070408, 0.4);
  csr.update(0.063140739, 1);
  csr.update(0.062858233, 0.6);
  csr.update(0.872580081, 1);
  csr.update(0.062307872, 0.9);
  csr.update(0.062040461, 1);
  csr.update(0.874968456, 0.6);
  csr.update(0.061517802, 0.9);
  csr.update(0.876488363, 1);
  csr.update(0.061012235, 0.6);
  csr.update(0.060765504, 0.9);
  csr.update(43.75649419, 0.4);
  csr.update(0.879389936, 1);
  csr.update(0.880083546, 1);
  csr.update(0.8807676, 1);
  csr.update(43.91880613, 0.9);

  double lb = csr.lower_bound();
  double ub = csr.upper_bound();

  // Compare to test_confidence_sequence_robust.py
  EXPECT_NEAR(lb, 0.30209151281846858, 1e-4);
  EXPECT_NEAR(ub, 0.90219143188334106, 1e-4);

  // Test brentq separately
  double s = 139.8326745448;
  double thres = 6.46147;
  std::map<uint64_t, double> memo;
  memo[0] = 39.016179559463588;
  memo[1] = 20.509121588503511;
  memo[2] = 10.821991705197142;
  double min_mu = 0.0;
  double max_mu = 1.0;

  // Get root
  double root = csr.lower.root_bisect(s, thres, memo, min_mu, max_mu);

  // Compare root to test_confidence_sequence_robust.py
  EXPECT_NEAR(root, 0.30209143008131789, 1e-4);

  // Test that root of objective function is 0
  auto test_root = csr.lower.log_wealth_mix(root, s, thres, memo) - thres;
  EXPECT_NEAR(test_root, 0.0, 1e-2);
}
