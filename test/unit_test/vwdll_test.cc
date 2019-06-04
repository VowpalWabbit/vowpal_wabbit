
#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>

#include "vwdll.h"
#include "vw.h"

template<class T>
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

BOOST_AUTO_TEST_CASE(vw_dll_parsed_and_constructed_example_parity)
{
  //parse example
  VW_HANDLE handle1 = VW_InitializeA("-q st --noconstant --quiet");
  VW_EXAMPLE example_parsed;
  example_parsed = VW_ReadExampleA(handle1, "1 |s p^the_man w^the w^man |t p^un_homme w^un w^homme");

  //construct example
  VW_HANDLE handle2 = VW_InitializeA("-q st --noconstant --quiet");
  VW_EXAMPLE example_constructed;
  auto fs = VW_InitializeFeatureSpace(handle2, 2);

  auto first = VW_GetPrimitiveFeatureSpace(fs, 0);
  VW_InitFeatures(first, 3);
  VW_SetFeatureSpaceA(first, "s");
  VW_SetFeatureA(first, 0, "p^the_man", 1.0f);
  VW_SetFeatureA(first, 1, "w^the", 1.0f);
  VW_SetFeatureA(first, 2, "w^man",  1.0f);

  auto second = VW_GetPrimitiveFeatureSpace(fs, 1);
  VW_InitFeatures(second, 3);
  VW_SetFeatureSpaceA(second, "t");
  VW_SetFeatureA(second, 0, "p^un_homme", 1.0f);
  VW_SetFeatureA(second, 1, "w^un", 1.0f);
  VW_SetFeatureA(second, 2, "w^homme", 1.0f);

  example_constructed = VW_ImportExample(handle2, "1", fs);


  // learn both
  auto score_parsed = VW_Learn(handle1, example_parsed);
  auto score_constructed = VW_Learn(handle2, example_parsed);


  //check parity
  BOOST_CHECK_EQUAL(score_parsed, score_constructed);
  auto vw1 = static_cast<vw*>(handle1);
  auto vw2 = static_cast<vw*>(handle2);

  BOOST_CHECK_EQUAL(vw1->weights.sparse, vw2->weights.sparse);

  if (vw1->weights.sparse) {
    check_weights_equal(vw1->weights.sparse_weights, vw2->weights.sparse_weights);
  }
  else {
    check_weights_equal(vw1->weights.dense_weights, vw2->weights.dense_weights);
  }

  VW_ReleaseFeatureSpace(fs, 2);

  VW_Finish(handle1);
  VW_Finish(handle2);
}


