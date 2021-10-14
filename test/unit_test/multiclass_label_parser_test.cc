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

void parse_label(label_parser& lp, parser* p, shared_data* sd, VW::string_view label, polylabel& l)
{
  tokenize(' ', label, p->words);
  lp.default_label(&l);
  reduction_features red_fts;
  lp.parse_label(p, sd, &l, p->words, red_fts);
}

BOOST_AUTO_TEST_CASE(multiclass_label_parser)
{
  auto lp = MULTICLASS::mc_label;
  parser p{8 /*ring_size*/, false /*strict parse*/};
  shared_data sd;

  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, &sd, "1,2,3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, &sd, "1a", *plabel), VW::vw_exception);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, &sd, "1 2 3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    parse_label(lp, &p, &sd, "2", *plabel);
    BOOST_ASSERT(plabel->multi.label == 2);
    BOOST_ASSERT(plabel->multi.weight == 1.0);
  }
  {
    auto plabel = scoped_calloc_or_throw<polylabel>();
    parse_label(lp, &p, &sd, "2 2", *plabel);
    BOOST_ASSERT(plabel->multi.label == 2);
    BOOST_ASSERT(plabel->multi.weight == 2.0);
  }
}
