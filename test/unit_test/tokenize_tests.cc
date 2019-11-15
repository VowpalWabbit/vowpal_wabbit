#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "parse_primitives.h"
#include "vw.h"

#include <vector>
#include <string>
#include <cstring>

BOOST_AUTO_TEST_CASE(tokenize_basic_string) {
  std::vector<VW::string_view> container;
  tokenize(' ', "this is   a string  ", container);

  auto const expected_values = {"this", "is", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty) {
  std::vector<VW::string_view> container;
  tokenize(' ', "this is   a string  ", container, true);

  auto const expected_values = {"this", "is","", "", "a", "string", "", ""};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty_no_end_space) {
  std::vector<VW::string_view> container;
  tokenize(' ', "this is   a string", container, true);

  auto const expected_values = {"this", "is","", "", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_escaped) {
  auto const container = escaped_tokenize(' ', "this is   a string  ");

  auto const expected_values = {"this", "is", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty_escaped) {
  auto const container = escaped_tokenize(' ', "this is   a string  ", true);

  auto const expected_values = {"this", "is","", "", "a", "string", "", ""};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty_no_end_space_escaped) {
  auto const container = escaped_tokenize(' ', "this is   a string", true);

  auto const expected_values = {"this", "is","", "", "a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_with_slash_escaped) {
  auto const container = escaped_tokenize(' ', "this is\\ a string");

  auto const expected_values = {"this", "is a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_with_doubleslash_escaped) {
  auto const container = escaped_tokenize(' ', "this is\\\\ a string");

  auto const expected_values = {"this", "is\\" ,"a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_with_normal_char_slash_escaped) {
  auto const container = escaped_tokenize(' ', "this \\is a string");

  auto const expected_values = {"this", "is" ,"a", "string"};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    container.begin(), container.end(),
    expected_values.begin(), expected_values.end());
}

BOOST_AUTO_TEST_CASE(tokenize_to_argv_with_space) {
  int argc = 0;
  auto argv = VW::to_argv("--ring_size 1024 -f my_model\\ best.model", argc);
  BOOST_CHECK_EQUAL(argc, 5);
  BOOST_CHECK(strcmp(argv[0], "b") == 0);
  BOOST_CHECK(strcmp(argv[1], "--ring_size") == 0);
  BOOST_CHECK(strcmp(argv[2], "1024") == 0);
  BOOST_CHECK(strcmp(argv[3], "-f") == 0);
  BOOST_CHECK(strcmp(argv[4], "my_model best.model") == 0);
}
