// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/text_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(WrapText, WrapText)
{
  auto result = VW::wrap_text("test", 10);
  auto expected = R"(test)";
  EXPECT_EQ(expected, result);

  result = VW::wrap_text("test another word with", 1);
  expected = R"(test
another
word
with)";
  EXPECT_EQ(expected, result);

  result = VW::wrap_text(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer non neque massa. In placerat ligula placerat "
      "ullamcorper egestas.",
      25);
  expected = R"(Lorem ipsum dolor sit amet,
consectetur adipiscing elit.
Integer non neque massa. In
placerat ligula placerat ullamcorper
egestas.)";
  EXPECT_EQ(expected, result);
}