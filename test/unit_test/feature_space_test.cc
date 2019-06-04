#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>

#include "vw.h"

template <class T>
void check_weights_equal(T& first, T& second)
{
  auto secondIt = second.begin();
  for (auto firstIt : first)
  {
    BOOST_CHECK_EQUAL(firstIt, *secondIt);
    ++secondIt;
  }
  BOOST_CHECK_EQUAL(secondIt == second.end(), true);
}

BOOST_AUTO_TEST_CASE(parsed_and_constructed_example_parity)
{
  vw* vw1 = VW::initialize("-q st --noconstant --quiet");
  vw* vw2 = VW::initialize("-q st --noconstant --quiet");

  auto example_parsed = VW::read_example(*vw1, "1 |s p^the_man w^the w^man |t p^un_homme w^un w^homme");
  auto fs = VW::feature_space(*vw2, 2);
  fs[0]
    .reset(3)
    .set_name("s")
    .set(0, "p^the_man", 1.0f)
    .set(1, "w^the", 1.0f)
    .set(2, "w^man", 1.0f);

   fs[1]
    .reset(3)
    .set_name("t")
    .set(0, "p^un_homme", 1.0f)
    .set(1, "w^un", 1.0f)
    .set(2, "w^homme", 1.0f);

  auto example_constructed = VW::import_example(*vw2, "1", fs);

  vw1->learn(*example_parsed);
  vw2->learn(*example_constructed);

  BOOST_CHECK_EQUAL(vw1->weights.sparse, vw2->weights.sparse);

  if (vw1->weights.sparse)
  {
    check_weights_equal(vw1->weights.sparse_weights, vw2->weights.sparse_weights);
  }
  else
  {
    check_weights_equal(vw1->weights.dense_weights, vw2->weights.dense_weights);
  }

  VW::finish(*vw1);
  VW::finish(*vw2);
}
