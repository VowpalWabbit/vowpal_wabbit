
#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>

#include "distributionally_robust.h"

BOOST_AUTO_TEST_CASE(distributionally_robust_inverse_chisq, *boost::unit_test::tolerance(1e-5))
{
  // Table[{ alpha, InverseCDF[ChiSquareDistribution[1], 1 - alpha] }, { alpha, 0.001, 0.501, 0.05 }]

  std::pair<double, double> testcases[] = {{0.001, 10.8276}, {0.051, 3.80827}, {0.101, 2.68968}, {0.151, 2.06212},
      {0.201, 1.63509}, {0.251, 1.31773}, {0.301, 1.06976}, {0.351, 0.869839}, {0.401, 0.705326}, {0.451, 0.568137},
      {0.501, 0.452817}};

  for (auto pt : testcases)
  {
    double v = VW::distributionally_robust::ChiSquared::chisq_onedof_isf(pt.first);

    BOOST_TEST(v == pt.second);
  }
}

BOOST_AUTO_TEST_CASE(distributionally_robust_recompute_duals, *boost::unit_test::tolerance(1e-5))
{
  // to generate this data:
  // 
  // python ./DistributionallyRobustUnitTestData.py

  std::pair<double, double> data[] = {{0.4692680899768591, 0.08779271803562538},
      {3.010121430917521, 0.1488852932982503}, {1.3167456935454493, 0.5579699034039329},
      {0.9129425537759532, 0.6202896863254631}, {0.16962487046234628, 0.7284609393916186},
      {0.16959629191460518, 0.10028857734263497}, {0.059838768608680676, 0.5165390922355259},
      {2.0112308644799395, 0.4596272470443479}, {0.9190821536272645, 0.4352012556681023},
      {1.2312500617045903, 0.40365563207132593}};

  VW::distributionally_robust::ChiSquared::Duals duals[] = {{true, 0, 0, 0, 1},
      {false, 0.186284935714629, -0.5242563567278763, 0, 1.999},
      {false, 0.24176630719751424, -0.3939735949427358, -0.1283677781597634, 2.997001},
      {false, 0.2789701026811336, -0.5061803928309371, -0.11471449055314126, 3.994003999},
      {false, 0.28140131203664326, -0.43475483491188227, -0.16912405473103076, 4.9900099950009995},
      {false, 0.29153800750906095, -0.3748233156965521, -0.22291421513333443, 5.985019985005999},
      {false, 0.37075115114133017, -0.7039218308392182, 0, 6.979034965020992},
      {false, 0.563270986745603, -0.9948022925290081, 0, 7.972055930055971},
      {false, 0.5602916549924593, -0.9880343238436395, 0, 8.964083874125915},
      {false, 0.5684515391242324, -1.0040272155608332, 0, 9.95511979025179}};

  auto onlinechisq = std::make_unique<VW::distributionally_robust::ChiSquared>(0.05, 0.999);

  for (int i = 0; i < std::extent<decltype(data)>::value; ++i)
  {
    onlinechisq->update(data[i].first, data[i].second);
    auto d = onlinechisq->recompute_duals();

    BOOST_TEST(duals[i].unbounded == d.unbounded);
    BOOST_TEST(duals[i].kappa == d.kappa);
    BOOST_TEST(duals[i].gamma == d.gamma);
    BOOST_TEST(duals[i].beta == d.beta);
    BOOST_TEST(duals[i].n == d.n);
  }
}

BOOST_AUTO_TEST_CASE(distributionally_robust_qlb, *boost::unit_test::tolerance(2e-5))
{
  // to generate this data:
  // 
  // python ./DistributionallyRobustUnitTestData.py

  std::pair<double, double> data[] = {{0.4692680899768591, 0.08779271803562538},
      {3.010121430917521, 0.1488852932982503}, {1.3167456935454493, 0.5579699034039329},
      {0.9129425537759532, 0.6202896863254631}, {0.16962487046234628, 0.7284609393916186},
      {0.16959629191460518, 0.10028857734263497}, {0.059838768608680676, 0.5165390922355259},
      {2.0112308644799395, 0.4596272470443479}, {0.9190821536272645, 0.4352012556681023},
      {1.2312500617045903, 0.40365563207132593}};

  double qlbs[] = {1, 0.13620517641052662, -0.17768396518176874, 0.03202698335276157, 0.20163624787093867,
      0.19427440609482105, 0.22750472815940542, 0.01392757858090217, 0.10533233309112934, 0.08141788541188416};

  auto onlinechisq = std::make_unique<VW::distributionally_robust::ChiSquared>(0.05, 0.999);

  for (int i = 0; i < std::extent<decltype(data)>::value; ++i)
  {
    onlinechisq->update(data[i].first, data[i].second);
    auto v = onlinechisq->qlb(data[i].first, data[i].second);

    BOOST_TEST(qlbs[i] == v);
  }
}
