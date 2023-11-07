// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

using namespace ::testing;

TEST(Tokenize, TokenizeBasicStringEscaped)
{
  std::string str = "this is   a string  ";
  auto const container = VW::details::escaped_tokenize(' ', str);
  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq("a"), StrEq("string")));
}

TEST(Tokenize, TokenizeBasicStringAllowEmptyEscaped)
{
  std::string str = "this is   a string  ";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container,
      ElementsAre(StrEq("this"), StrEq("is"), StrEq(""), StrEq(""), StrEq("a"), StrEq("string"), StrEq(""), StrEq("")));
}

TEST(Tokenize, TokenizeBasicStringAllowEmptyNoEndSpaceEscaped)
{
  std::string str = "this is   a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq(""), StrEq(""), StrEq("a"), StrEq("string")));
}

TEST(Tokenize, TokenizeBasicStringWithSlashEscaped)
{
  std::string str = "this is\\ a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is a"), StrEq("string")));
}

TEST(Tokenize, TokenizeBasicStringWithDoubleslashEscaped)
{
  std::string str = "this is\\\\ a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is\\"), StrEq("a"), StrEq("string")));
}

TEST(Tokenize, TokenizeBasicStringWithNormalCharSlashEscaped)
{
  std::string str = "this \\is a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);
  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq("a"), StrEq("string")));
}

TEST(Tokenize, TokenizeBasicStringWithEscapeFinalCharacter)
{
  std::string str = "this is a string\\";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq("a"), StrEq("string")));
}

TEST(Tokenize, TokenizeToArgvWithSpace)
{
  int argc = 0;
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  char** argv = VW::to_argv_escaped("--example_queue_limit 1024 -f my_model\\ best.model", argc);
  VW_WARNING_STATE_POP

  EXPECT_EQ(argc, 5);
  EXPECT_STREQ(argv[0], "b");
  EXPECT_STREQ(argv[1], "--example_queue_limit");
  EXPECT_STREQ(argv[2], "1024");
  EXPECT_STREQ(argv[3], "-f");
  EXPECT_STREQ(argv[4], "my_model best.model");

  for (int i = 0; i < argc; i++) { free(argv[i]); }
  free(argv);
}

TEST(Tokenize, BasicTokenizeToArgv)
{
  int argc = 0;
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  char** argv = VW::to_argv_escaped("--ccb_explore_adf --json --no_stdin --quiet", argc);
  VW_WARNING_STATE_POP

  EXPECT_EQ(argc, 5);
  EXPECT_STREQ(argv[0], "b");
  EXPECT_STREQ(argv[1], "--ccb_explore_adf");
  EXPECT_STREQ(argv[2], "--json");
  EXPECT_STREQ(argv[3], "--no_stdin");
  EXPECT_STREQ(argv[4], "--quiet");

  for (int i = 0; i < argc; i++) { free(argv[i]); }
  free(argv);
}

TEST(Tokenize, EscapedSplitCommandLineTest)
{
  auto args = VW::split_command_line(VW::string_view(R"(--example_queue_limit 1024 -f my_model\ best.model)"));
  EXPECT_THAT(
      args, ElementsAre(StrEq("--example_queue_limit"), StrEq("1024"), StrEq("-f"), StrEq("my_model best.model")));
}

TEST(Tokenize, BasicSplitCommandLine)
{
  auto args = VW::split_command_line(VW::string_view(R"(--ccb_explore_adf --json --no_stdin --quiet)"));
  EXPECT_THAT(args, ElementsAre(StrEq("--ccb_explore_adf"), StrEq("--json"), StrEq("--no_stdin"), StrEq("--quiet")));
}

TEST(Tokenize, ComplexSplitCommandLine)
{
  auto args = VW::split_command_line(VW::string_view(R"(-d "this is my file\"" 'another arg' test\ arg \\test)"));
  EXPECT_THAT(args,
      ElementsAre(StrEq("-d"), StrEq("this is my file\""), StrEq("another arg"), StrEq("test arg"), StrEq(R"(\test)")));
}

TEST(Tokenize, UnclosedQuoteSplitCommandLine)
{
  EXPECT_THROW(VW::split_command_line(VW::string_view(R"(my arg "with strings)")), VW::vw_exception);
}

TEST(Tokenize, EscapedEndSplitCommandLine)
{
  EXPECT_THROW(VW::split_command_line(VW::string_view(R"(my arg \)")), VW::vw_exception);
}

TEST(Tokenize, MixedQuotesSplitCommandLine)
{
  auto args = VW::split_command_line(VW::string_view(R"("this is 'a quoted'" '"unclosed')"));
  EXPECT_THAT(args, ElementsAre(StrEq("this is 'a quoted'"), StrEq("\"unclosed")));
}
