// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/text_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

using namespace ::testing;

TEST(Tokenize, TokenizeBasicString)
{
  std::vector<VW::string_view> container;
  std::string str = "this is   a string  ";
  VW::tokenize(' ', str, container);
  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq("a"), StrEq("string")));
}

TEST(Tokenize, TokenizeBasicStringAllowEmpty)
{
  std::vector<VW::string_view> container;
  std::string str = "this is   a string  ";
  VW::tokenize(' ', str, container, true);

  EXPECT_THAT(container,
      testing::ElementsAre(
          StrEq("this"), StrEq("is"), StrEq(""), StrEq(""), StrEq("a"), StrEq("string"), StrEq(""), StrEq("")));
}

TEST(Tokenize, TokenizeBasicStringAllowEmptyNoEndSpace)
{
  std::vector<VW::string_view> container;
  std::string str = "this is   a string";
  VW::tokenize(' ', str, container, true);

  EXPECT_THAT(
      container, testing::ElementsAre(StrEq("this"), StrEq("is"), StrEq(""), StrEq(""), StrEq("a"), StrEq("string")));
}
