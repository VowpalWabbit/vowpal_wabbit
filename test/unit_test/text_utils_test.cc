// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>

#include "vw/common/text_utils.h"

BOOST_AUTO_TEST_CASE(wrap_text_tests)
{
  auto result = VW::common::wrap_text("test", 10);
  auto expected = R"(test)";
  BOOST_CHECK_EQUAL(expected, result);

  result = VW::common::wrap_text("test another word with", 1);
  expected = R"(test
another
word
with)";
  BOOST_CHECK_EQUAL(expected, result);

  result = VW::common::wrap_text(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer non neque massa. In placerat ligula placerat "
      "ullamcorper egestas.",
      25);
  expected = R"(Lorem ipsum dolor sit amet,
consectetur adipiscing elit.
Integer non neque massa. In
placerat ligula placerat ullamcorper
egestas.)";
  BOOST_CHECK_EQUAL(expected, result);
}
