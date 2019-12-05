
#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>

#include "distributionally_robust.h"

BOOST_AUTO_TEST_CASE(distributionally_robust_inverse_chisq, * boost::unit_test::tolerance(1e-5))
{
  std::pair<double, double> testcases[] = {{0.001, 10.8276}, {0.051, 3.80827}, {0.101, 2.68968}, {0.151, 2.06212},
      {0.201, 1.63509}, {0.251, 1.31773}, {0.301, 1.06976}, {0.351, 0.869839}, {0.401, 0.705326}, {0.451, 0.568137},
      {0.501, 0.452817}};


  for (auto pt : testcases)
  {
    double v = VW::distributionally_robust::ChiSquared::chisq_onedof_isf(pt.first);

    BOOST_TEST(v == pt.second);
  }
}
