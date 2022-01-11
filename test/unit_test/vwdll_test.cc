// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>

#include "vwdll.h"
#include "vw.h"
#include "test_common.h"
#include "vw_string_view.h"

using namespace boost::unit_test;

template<class T>
void check_weights_equal(T& first, T& second)
{
  BOOST_CHECK_EQUAL_COLLECTIONS(first.begin(), first.end(), second.begin(), second.end());
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
  auto vw1 = static_cast<VW::workspace*>(handle1);
  auto vw2 = static_cast<VW::workspace*>(handle2);

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

// This test seems to have issues on the older MSVC compiler CI, but no issues in the newer.
#if (defined(_MSC_VER) && (_MSC_VER >= 1920)) || !defined(_MSC_VER)

BOOST_AUTO_TEST_CASE(vw_dll_get_audit_output)
{
  // parse example
  VW_HANDLE handle = VW_InitializeA("--noconstant --quiet --audit");
  VW_CaptureAuditData(handle);
  VW_EXAMPLE example_parsed;
  example_parsed = VW_ReadExampleA(handle, "1 | test example");
  VW_Learn(handle, example_parsed);

  size_t audit_size = 0;
  char* audit_data = VW_GetAuditDataA(handle, &audit_size);

  VW::string_view expected_audit = R"(0
	test:250387:1:0@0	example:99909:1:0@0
)";
  VW::string_view audit_data_view(audit_data, audit_size);
  BOOST_CHECK_EQUAL(audit_data_view, expected_audit);

  VW_FreeAuditDataA(handle, audit_data);
  VW_FinishExample(handle, example_parsed);
  VW_Finish(handle);
}

#endif

#ifndef __APPLE__
BOOST_AUTO_TEST_CASE(vw_dll_parse_escaped)
{
  // This call doesn't escape and so sees --nonexistent_option as a standalone invalid argument.
  BOOST_CHECK_EQUAL(VW_InitializeA("--id test\\ --nonexistent_option --quiet"), nullptr);

  // The space is escaped and so the data argument becomes "test --nonexistent_option"
  VW_HANDLE handle1 = VW_InitializeEscapedA("--id test\\ --nonexistent_option --quiet");
  BOOST_CHECK(handle1 != nullptr);
  VW_Finish(handle1);
}
#endif
