// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/estimators/confidence_sequence.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

constexpr float CS_FLOAT_TOL = 0.1f;

TEST(ConfidenceSeq, IncrementalFsumTest)
{
  VW::details::incremental_f_sum fsum;
  fsum += 1.0;
  fsum += 2.0;
  fsum += 3.0;
  fsum += 4.0;
  fsum += 5.0;
  fsum += 6.0;
  fsum += 7.0;
  fsum += 8.0;
  fsum += 9.0;
  fsum += 10.0;
  EXPECT_FLOAT_EQ(fsum, 55.0);
}

TEST(ConfidenceSeq, ConfidenceSequenceTest)
{
  VW::estimators::confidence_sequence cs;
  std::vector<double> rs;
  for (int i = 0; i < 1000; ++i)
  {
    rs.push_back(0.5);
    rs.push_back(0.6);
    rs.push_back(0.7);
    rs.push_back(0.8);
  }

  for (double r : rs) { cs.update(r, r); }

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();

  // Compare to test_confidence_sequence.py
  EXPECT_NEAR(lb, 0.4215480f, CS_FLOAT_TOL);
  EXPECT_NEAR(ub, 0.7907692f, CS_FLOAT_TOL);
}