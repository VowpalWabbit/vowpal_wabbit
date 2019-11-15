#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "parse_primitives.h"

#include <vector>
#include <string>

BOOST_AUTO_TEST_CASE(tokenize_basic_string) {
  std::vector<substring> container;
  std::string str = "this is   a string  ";
  substring ss = {const_cast<char*>(str.c_str()), const_cast<char*>(str.c_str()) + str.size()};
  tokenize(' ', ss, container);

  BOOST_CHECK_EQUAL(container.size(), 4);
  BOOST_CHECK(substring_equal(container[0], "this"));
  BOOST_CHECK(substring_equal(container[1], "is"));
  BOOST_CHECK(substring_equal(container[2], "a"));
  BOOST_CHECK(substring_equal(container[3], "string"));
}

BOOST_AUTO_TEST_CASE(tokenize_basic_string_allow_empty) {
  std::vector<substring> container;
  std::string str = "this is   a string  ";
  substring ss = {const_cast<char*>(str.c_str()), const_cast<char*>(str.c_str()) + str.size()};
  tokenize(' ', ss, container, true);

  BOOST_CHECK_EQUAL(container.size(), 8);
  BOOST_CHECK(substring_equal(container[0], "this"));
  BOOST_CHECK(substring_equal(container[1], "is"));
  BOOST_CHECK(substring_equal(container[2], ""));
  BOOST_CHECK(substring_equal(container[3], ""));
  BOOST_CHECK(substring_equal(container[4], "a"));
  BOOST_CHECK(substring_equal(container[5], "string"));
  BOOST_CHECK(substring_equal(container[6], ""));
  BOOST_CHECK(substring_equal(container[7], ""));
}

BOOST_AUTO_TEST_CASE(tokenize_quoted_string) {
  std::vector<substring> container;
  std::string str = "this is\\ a \\\\string";
  substring ss = {const_cast<char*>(str.c_str()), const_cast<char*>(str.c_str()) + str.size()};
  tokenize(' ', ss, container, false, true);

  BOOST_CHECK_EQUAL(container.size(), 3);
  BOOST_CHECK(substring_equal(container[0], "this"));
  BOOST_CHECK(substring_equal(container[1], "is a"));
  BOOST_CHECK(substring_equal(container[3], "\\string"));
}

// BOOST_AUTO_TEST_CASE(tokenize_quoted_string_no_end) {
//   std::vector<substring> container;
//   std::string str = "this is   a string  ";
//   substring ss = {const_cast<char*>(str.c_str()), const_cast<char*>(str.c_str()) + str.size()};
//   tokenize(' ', ss, container, true);

//   BOOST_CHECK_EQUAL(container.size(), 8);
//   BOOST_CHECK(substring_equal(container[0], "this"));
// }

