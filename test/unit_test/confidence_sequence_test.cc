// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence.h"

#include "test_common.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

constexpr float CS_FLOAT_TOL = 0.1f;

BOOST_AUTO_TEST_CASE(incremental_fsum_test)
{
  VW::confidence_sequence::IncrementalFsum fsum;
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
  BOOST_CHECK_EQUAL(fsum, 55.0);
}

BOOST_AUTO_TEST_CASE(confidence_sequence_test)
{
  VW::confidence_sequence::ConfidenceSequence cs;
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

  BOOST_CHECK_CLOSE(lb, 0.4215480f, CS_FLOAT_TOL);
  BOOST_CHECK_CLOSE(ub, 0.7907692f, CS_FLOAT_TOL);
}