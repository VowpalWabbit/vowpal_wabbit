// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"
#include <boost/math/tools/minima.hpp>

#include "test_common.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(confidence_sequence_robust_test)
{
    VW::off_policy_cs opcs;
    for (int i = 0; i < 200; ++i)
    {
        opcs.add_obs(1.1, 1);
        opcs.add_obs(1.1, 0);
        auto ci = opcs.get_ci(.05);
        std::cout << ci.first << " " << ci.second << "\n";
    }
}

