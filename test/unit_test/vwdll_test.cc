
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
  auto fs = VW_InitializeFeatureSpaces(2);

  auto first = VW_GetFeatureSpace(fs, 0);
  VW_InitFeatures(first, 3);
  auto shash = VW_SetFeatureSpace(handle2, first, "s");
  VW_SetFeature(first, 0, VW_HashFeatureA(handle2, "p^the_man", shash), 1.0f);
  VW_SetFeature(first, 1, VW_HashFeatureA(handle2, "w^the", shash), 1.0f);
  VW_SetFeature(first, 2, VW_HashFeatureA(handle2, "w^man", shash), 1.0f);

  auto second = VW_GetFeatureSpace(fs, 1);
  VW_InitFeatures(second, 3);
  auto thash = VW_SetFeatureSpace(handle2, second, "t");
  VW_SetFeature(second, 0, VW_HashFeatureA(handle2, "p^un_homme", thash), 1.0f);
  VW_SetFeature(second, 1, VW_HashFeatureA(handle2, "w^un", thash), 1.0f);
  VW_SetFeature(second, 2, VW_HashFeatureA(handle2, "w^homme", thash), 1.0f);

  example_constructed = VW_ImportExample(handle2, "1", fs, 2);


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

  VW_FinishExample(handle1, example_parsed);
  VW_FinishExample(handle2, example_constructed);

  VW_Finish(handle1);
  VW_Finish(handle2);
}

BOOST_AUTO_TEST_CASE(vw_dll_parse_escaped)
{
  // This call doesn't escape and so sees --nonexistent_option as a standalone invalid argument.
  BOOST_CHECK_THROW(VW_InitializeA("-d test\\ --nonexistent_option --quiet"), VW::vw_unrecognised_option_exception);

  // The space is escaped and so the data argument becomes "test --nonexistent_option"
  VW_HANDLE handle1 = VW_InitializeEscapedA("-d test\\ --nonexistent_option --quiet");
  BOOST_CHECK(handle1 != nullptr);
  VW_Finish(handle1);
}
