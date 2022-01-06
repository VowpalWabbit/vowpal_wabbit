// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "parse_primitives.h"
#include "multiclass.h"
#include "parser.h"
#include "global_data.h"
#include "shared_data.h"
#include "vw_string_view.h"

void parse_label(label_parser& lp, VW::string_view label, polylabel& l)
{
  std::vector<VW::string_view> words;
  tokenize(' ', label, words);
  lp.default_label(l);
  reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}

BOOST_AUTO_TEST_CASE(multiclass_label_parser)
{
  auto lp = MULTICLASS::mc_label;
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    BOOST_REQUIRE_THROW(parse_label(lp, "1,2,3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    BOOST_REQUIRE_THROW(parse_label(lp, "1a", *plabel), VW::vw_exception);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    BOOST_REQUIRE_THROW(parse_label(lp, "1 2 3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    parse_label(lp, "2", *plabel);
    BOOST_ASSERT(plabel->multi.label == 2);
    BOOST_ASSERT(plabel->multi.weight == 1.0);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    parse_label(lp, "2 2", *plabel);
    BOOST_ASSERT(plabel->multi.label == 2);
    BOOST_ASSERT(plabel->multi.weight == 2.0);
  }
}