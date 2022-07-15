// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/example.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(example_move_ctor_moves_pred)
{
  VW::example ex;
  ex.pred.scalars.push_back(8);
  BOOST_CHECK_EQUAL(ex.pred.scalars.size(), 1);

  VW::example ex2(std::move(ex));

  BOOST_CHECK_EQUAL(ex.pred.scalars.size(), 0);
  BOOST_CHECK_EQUAL(ex2.pred.scalars.size(), 1);
}

BOOST_AUTO_TEST_CASE(example_move_assign_op_moves_pred)
{
  VW::example ex;
  ex.pred.a_s.push_back({0, 0});
  BOOST_CHECK_EQUAL(ex.pred.a_s.size(), 1);

  VW::example ex2;
  BOOST_CHECK_EQUAL(ex2.pred.a_s.size(), 0);
  ex2 = std::move(ex);

  BOOST_CHECK_EQUAL(ex.pred.a_s.size(), 0);
  BOOST_CHECK_EQUAL(ex2.pred.a_s.size(), 1);
}
