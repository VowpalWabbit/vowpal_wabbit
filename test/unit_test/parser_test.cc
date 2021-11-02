// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "parse_args.h"
#include "parse_example.h"
#include "parse_primitives.h"

BOOST_AUTO_TEST_CASE(decode_inline_hex_test)
{
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("test"), "test");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("10"), "10");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("\\x01"), "\x01");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("\\xab"), "\xab");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("\\x01 unrelated \\x56"), "\x01 unrelated \x56");
}

BOOST_AUTO_TEST_CASE(parse_text_with_extents)
{
  auto* vw = VW::initialize("--no_stdin --quiet", nullptr, false, nullptr, nullptr);
  auto* ex = VW::read_example(*vw, "|features a b |new_features a b |features2 c d |empty |features c d");

  BOOST_CHECK_EQUAL(ex->feature_space['f'].size(), 6);
  BOOST_CHECK_EQUAL(ex->feature_space['n'].size(), 2);
  BOOST_CHECK_EQUAL(ex->feature_space['3'].size(), 0);

  BOOST_CHECK_EQUAL(ex->feature_space['f'].namespace_extents.size(), 3);
  BOOST_CHECK_EQUAL(ex->feature_space['n'].namespace_extents.size(), 1);

  BOOST_CHECK_EQUAL(
      ex->feature_space['f'].namespace_extents[0], (VW::namespace_extent{0, 2, VW::hash_space(*vw, "features")}));
  BOOST_CHECK_EQUAL(
      ex->feature_space['f'].namespace_extents[1], (VW::namespace_extent{2, 4, VW::hash_space(*vw, "features2")}));
  BOOST_CHECK_EQUAL(
      ex->feature_space['f'].namespace_extents[2], (VW::namespace_extent{4, 6, VW::hash_space(*vw, "features")}));

  VW::finish_example(*vw, *ex);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(trim_whitespace_test)
{
  BOOST_TEST("" == VW::trim_whitespace(VW::string_view("")));
  BOOST_TEST("abc" == VW::trim_whitespace(VW::string_view("abc")));
  BOOST_TEST("abc" == VW::trim_whitespace(VW::string_view("              abc               ")));
  BOOST_TEST("ab     c" == VW::trim_whitespace(VW::string_view("              ab     c               ")));
  BOOST_TEST("a\nb     c" == VW::trim_whitespace(VW::string_view("              a\nb     c               ")));
  BOOST_TEST("a\nb     \tc" == VW::trim_whitespace(VW::string_view("     \t         a\nb     \tc        \t\t       ")));
  BOOST_TEST("" == VW::trim_whitespace(VW::string_view("     \t                 \t\t       ")));

  BOOST_TEST("" == VW::trim_whitespace(std::string("")));
  BOOST_TEST("abc" == VW::trim_whitespace(std::string("abc")));
  BOOST_TEST("abc" == VW::trim_whitespace(std::string("              abc               ")));
  BOOST_TEST("ab     c" == VW::trim_whitespace(std::string("              ab     c               ")));
  BOOST_TEST("a\nb     c" == VW::trim_whitespace(std::string("              a\nb     c               ")));
  BOOST_TEST("a\nb     \tc" == VW::trim_whitespace(std::string("     \t         a\nb     \tc        \t\t       ")));
  BOOST_TEST("" == VW::trim_whitespace(std::string("     \t                 \t\t       ")));
}