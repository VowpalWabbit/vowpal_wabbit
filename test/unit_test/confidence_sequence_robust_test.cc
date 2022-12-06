// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"
#include <boost/math/tools/minima.hpp>

#include "test_common.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(csr_test)
{
    double s = 748606.6666732753;
    double thres = 3.6888794541139363;
    std::map<uint64_t, double> memo;
    memo[0] = 49818.12424363672;
    memo[1] = 21075.040167550338;
    double min_mu = 0;
    double max_mu = 1;
    VW::countable_discrete_base cbd;
    auto lw_lambda = [](double mu) -> double { return mu*mu - mu + 5; };
    auto root = boost::math::tools::brent_find_minima(lw_lambda, min_mu, max_mu, 24);
    std::cout << cbd.root_brentq(s, thres, memo, min_mu, max_mu);
    //root: 0.736615800884814;
}

