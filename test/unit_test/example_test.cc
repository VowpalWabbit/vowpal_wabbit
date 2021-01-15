#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "example.h"

BOOST_AUTO_TEST_CASE(example_move_ctor_moves_pred)
{
  example ex;
  ex.pred.scalars.push_back(8);
  BOOST_CHECK_EQUAL(ex.pred.scalars.size(), 1);
  BOOST_CHECK_NE(ex.pred.scalars.cbegin(), nullptr);

  example ex2(std::move(ex));

  BOOST_CHECK_EQUAL(ex.pred.scalars.cbegin(), nullptr);
  BOOST_CHECK_EQUAL(ex2.pred.scalars.size(), 1);
}

BOOST_AUTO_TEST_CASE(example_move_assign_op_moves_pred)
{
  example ex;
  ex.pred.a_s.push_back({0, 0});
  BOOST_CHECK_EQUAL(ex.pred.a_s.size(), 1);
  BOOST_CHECK_NE(ex.pred.a_s.cbegin(), nullptr);

  example ex2;
  BOOST_CHECK_EQUAL(ex2.pred.a_s.cbegin(), nullptr);
  ex2 = std::move(ex);

  BOOST_CHECK_EQUAL(ex.pred.a_s.cbegin(), nullptr);
  BOOST_CHECK_EQUAL(ex2.pred.a_s.size(), 1);
}
