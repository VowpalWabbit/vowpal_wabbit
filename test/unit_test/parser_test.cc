// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "test_common.h"
#include "vw/core/parse_args.h"
#include "vw/core/parse_example.h"
#include "vw/core/parse_primitives.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(decode_inline_hex_test)
{
  auto nl = VW::io::create_null_logger();
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("test", nl), "test");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("10", nl), "10");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("\\x01", nl), "\x01");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("\\xab", nl), "\xab");
  BOOST_CHECK_EQUAL(VW::decode_inline_hex("\\x01 unrelated \\x56", nl), "\x01 unrelated \x56");
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
  BOOST_CHECK("" == VW::trim_whitespace(VW::string_view("")));
  BOOST_CHECK("abc" == VW::trim_whitespace(VW::string_view("abc")));
  BOOST_CHECK("abc" == VW::trim_whitespace(VW::string_view("              abc               ")));
  BOOST_CHECK("ab     c" == VW::trim_whitespace(VW::string_view("              ab     c               ")));
  BOOST_CHECK("a\nb     c" == VW::trim_whitespace(VW::string_view("              a\nb     c               ")));
  BOOST_CHECK(
      "a\nb     \tc" == VW::trim_whitespace(VW::string_view("     \t         a\nb     \tc        \t\t       ")));
  BOOST_CHECK("" == VW::trim_whitespace(VW::string_view("     \t                 \t\t       ")));
  BOOST_CHECK("" == VW::trim_whitespace(std::string("")));
  BOOST_CHECK("abc" == VW::trim_whitespace(std::string("abc")));
  BOOST_CHECK("abc" == VW::trim_whitespace(std::string("              abc               ")));
  BOOST_CHECK("ab     c" == VW::trim_whitespace(std::string("              ab     c               ")));
  BOOST_CHECK("a\nb     c" == VW::trim_whitespace(std::string("              a\nb     c               ")));
  BOOST_CHECK("a\nb     \tc" == VW::trim_whitespace(std::string("     \t         a\nb     \tc        \t\t       ")));
  BOOST_CHECK("" == VW::trim_whitespace(std::string("     \t                 \t\t       ")));
}