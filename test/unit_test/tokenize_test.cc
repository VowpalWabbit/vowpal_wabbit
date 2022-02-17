// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "parse_primitives.h"
#include "vw.h"

#include <vector>
#include <string>
#include <cstring>

BOOST_AUTO_TEST_CASE(tokenize_basic_string) {
  std::vector<VW::string_view> container;
  std::string str = "this is   a string  ";
  tokenize(' ', str, container);

  auto const expected_values = {"this", "is", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty) {
  std::vector<VW::string_view> container;
  std::string str = "this is   a string  ";
  tokenize(' ', str, container, true);

  auto const expected_values = {"this", "is","", "", "a", "string", "", ""};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty_no_end_space) {
  std::vector<VW::string_view> container;
  std::string str = "this is   a string";
  tokenize(' ', str, container, true);

  auto const expected_values = {"this", "is","", "", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_escaped) {
  std::string str = "this is   a string  ";
  auto const container = escaped_tokenize(' ', str);

  auto const expected_values = {"this", "is", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty_escaped) {
  std::string str = "this is   a string  ";
  auto const container = escaped_tokenize(' ', str, true);

  auto const expected_values = {"this", "is","", "", "a", "string", "", ""};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty_no_end_space_escaped) {
  std::string str = "this is   a string";
  auto const container = escaped_tokenize(' ', str, true);

  auto const expected_values = {"this", "is","", "", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_with_slash_escaped) {
  std::string str = "this is\\ a string";
  auto const container = escaped_tokenize(' ', str, true);

  auto const expected_values = {"this", "is a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_with_doubleslash_escaped) {
  std::string str = "this is\\\\ a string";
  auto const container = escaped_tokenize(' ', str, true);

  auto const expected_values = {"this", "is\\" ,"a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_with_normal_char_slash_escaped) {
  std::string str = "this \\is a string";
  auto const container = escaped_tokenize(' ', str, true);

  auto const expected_values = {"this", "is" ,"a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_with_escape_final_character) {
  std::string str = "this is a string\\";
  auto const container = escaped_tokenize(' ', str, true);

  auto const expected_values = {"this", "is" ,"a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_to_argv_with_space) {
  int argc = 0;
  char** argv = VW::to_argv_escaped("--example_queue_limit 1024 -f my_model\\ best.model", argc);
  BOOST_CHECK_EQUAL(argc, 5);
  BOOST_CHECK_EQUAL(argv[0], "b");
  BOOST_CHECK_EQUAL(argv[1], "--example_queue_limit");
  BOOST_CHECK_EQUAL(argv[2], "1024");
  BOOST_CHECK_EQUAL(argv[3], "-f");
  BOOST_CHECK_EQUAL(argv[4], "my_model best.model");

  for (int i = 0; i < argc; i++) free(argv[i]);
  free(argv);
}

BOOST_AUTO_TEST_CASE(basic_tokenize_to_argv) {
  int argc = 0;
  char** argv = VW::to_argv_escaped("--ccb_explore_adf --json --no_stdin --quiet", argc);

  BOOST_CHECK_EQUAL(argc, 5);
  BOOST_CHECK_EQUAL(argv[0], "b");
  BOOST_CHECK_EQUAL(argv[1], "--ccb_explore_adf");
  BOOST_CHECK_EQUAL(argv[2], "--json");
  BOOST_CHECK_EQUAL(argv[3], "--no_stdin");
  BOOST_CHECK_EQUAL(argv[4], "--quiet");

  for (int i = 0; i < argc; i++) free(argv[i]);
  free(argv);
}

BOOST_AUTO_TEST_CASE(escaped_split_command_line_test)
{
  auto args = VW::split_command_line(VW::string_view(R"(--example_queue_limit 1024 -f my_model\ best.model)"));
  const auto expected = {"--example_queue_limit", "1024", "-f", "my_model best.model"};
  BOOST_TEST(args == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(basic_split_command_line)
{
  auto args = VW::split_command_line(VW::string_view(R"(--ccb_explore_adf --json --no_stdin --quiet)"));
  const auto expected = {"--ccb_explore_adf", "--json", "--no_stdin", "--quiet"};
  BOOST_TEST(args == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(complex_split_command_line)
{
  auto args = VW::split_command_line(VW::string_view(R"(-d "this is my file\"" 'another arg' test\ arg \\test)"));
  const auto expected = {"-d", "this is my file\"", "another arg", "test arg", R"(\test)"};
  BOOST_TEST(args == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(complex_split_command_line_internal_quotes)
{
  auto args = VW::split_command_line(VW::string_view(R"(thisis"this is my file")"));
  const auto expected = {"thisisthis is my file"};
  BOOST_TEST(args == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(unclosed_quote_split_command_line)
{
  BOOST_CHECK_THROW(VW::split_command_line(VW::string_view(R"(my arg "with strings)")), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(escaped_end_split_command_line)
{
  BOOST_CHECK_THROW(VW::split_command_line(VW::string_view(R"(my arg \)")), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(mixed_quotes_split_command_line)
{
  auto args = VW::split_command_line(VW::string_view(R"("this is 'a quoted'" '"unclosed')"));
  const auto expected = {"this is 'a quoted'", "\"unclosed"};
  BOOST_TEST(args == expected, boost::test_tools::per_element());
}
