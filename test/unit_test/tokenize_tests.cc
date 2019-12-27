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
  auto argv = VW::to_argv_escaped("--ring_size 1024 -f my_model\\ best.model", argc);
  BOOST_CHECK_EQUAL(argc, 5);
  BOOST_CHECK_EQUAL(argv[0], "b");
  BOOST_CHECK_EQUAL(argv[1], "--ring_size");
  BOOST_CHECK_EQUAL(argv[2], "1024");
  BOOST_CHECK_EQUAL(argv[3], "-f");
  BOOST_CHECK_EQUAL(argv[4], "my_model best.model");
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
