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

TEST(tokenize, tokenize_basic_string_escaped)
{
  std::string str = "this is   a string  ";
  auto const container = VW::details::escaped_tokenize(' ', str);
  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq("a"), StrEq("string")));
}

TEST(tokenize, tokenize_basic_string_allow_empty_escaped)
{
  std::string str = "this is   a string  ";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container,
      ElementsAre(StrEq("this"), StrEq("is"), StrEq(""), StrEq(""), StrEq("a"), StrEq("string"), StrEq(""), StrEq("")));
}

TEST(tokenize, tokenize_basic_string_allow_empty_no_end_space_escaped)
{
  std::string str = "this is   a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq(""), StrEq(""), StrEq("a"), StrEq("string")));
}

TEST(tokenize, tokenize_basic_string_with_slash_escaped)
{
  std::string str = "this is\\ a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is a"), StrEq("string")));
}

TEST(tokenize, tokenize_basic_string_with_doubleslash_escaped)
{
  std::string str = "this is\\\\ a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is\\"), StrEq("a"), StrEq("string")));
}

TEST(tokenize, tokenize_basic_string_with_normal_char_slash_escaped)
{
  std::string str = "this \\is a string";
  auto const container = VW::details::escaped_tokenize(' ', str, true);
  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq("a"), StrEq("string")));
}

TEST(tokenize, tokenize_basic_string_with_escape_final_character)
{
  std::string str = "this is a string\\";
  auto const container = VW::details::escaped_tokenize(' ', str, true);

  EXPECT_THAT(container, ElementsAre(StrEq("this"), StrEq("is"), StrEq("a"), StrEq("string")));
}

TEST(tokenize, tokenize_to_argv_with_space)
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

TEST(tokenize, basic_tokenize_to_argv)
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

TEST(tokenize, escaped_split_command_line_test)
{
  auto args = VW::split_command_line(VW::string_view(R"(--example_queue_limit 1024 -f my_model\ best.model)"));
  EXPECT_THAT(
      args, ElementsAre(StrEq("--example_queue_limit"), StrEq("1024"), StrEq("-f"), StrEq("my_model best.model")));
}

TEST(tokenize, basic_split_command_line)
{
  auto args = VW::split_command_line(VW::string_view(R"(--ccb_explore_adf --json --no_stdin --quiet)"));
  EXPECT_THAT(args, ElementsAre(StrEq("--ccb_explore_adf"), StrEq("--json"), StrEq("--no_stdin"), StrEq("--quiet")));
}

TEST(tokenize, complex_split_command_line)
{
  auto args = VW::split_command_line(VW::string_view(R"(-d "this is my file\"" 'another arg' test\ arg \\test)"));
  EXPECT_THAT(args,
      ElementsAre(StrEq("-d"), StrEq("this is my file\""), StrEq("another arg"), StrEq("test arg"), StrEq(R"(\test)")));
}

TEST(tokenize, unclosed_quote_split_command_line)
{
  EXPECT_THROW(VW::split_command_line(VW::string_view(R"(my arg "with strings)")), VW::vw_exception);
}

TEST(tokenize, escaped_end_split_command_line)
{
  EXPECT_THROW(VW::split_command_line(VW::string_view(R"(my arg \)")), VW::vw_exception);
}

TEST(tokenize, mixed_quotes_split_command_line)
{
  auto args = VW::split_command_line(VW::string_view(R"("this is 'a quoted'" '"unclosed')"));
  EXPECT_THAT(args, ElementsAre(StrEq("this is 'a quoted'"), StrEq("\"unclosed")));
}
