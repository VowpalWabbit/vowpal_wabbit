#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "parse_primitives.h"
#include "multiclass.h"
#include "parser.h"
#include "global_data.h"

void parse_label(label_parser& lp, parser* p, shared_data* sd, VW::string_view label, MULTICLASS::label_t& l)
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
  auto sd = &calloc_or_throw<shared_data>();

  {
    MULTICLASS::label_t label;
    BOOST_REQUIRE_THROW(parse_label(lp, &p, sd, "1,2,3", label), VW::vw_exception);
  }
  {
    MULTICLASS::label_t label;
    BOOST_REQUIRE_THROW(parse_label(lp, &p, sd, "1a", label), VW::vw_exception);
  }
  {
    MULTICLASS::label_t label;
    BOOST_REQUIRE_THROW(parse_label(lp, &p, sd, "1 2 3", label), VW::vw_exception);
  }
  {
    MULTICLASS::label_t label;
    parse_label(lp, &p, sd, "2", label);
    BOOST_ASSERT(label.label == 2);
    BOOST_ASSERT(label.weight == 1.0);
  }
  {
    MULTICLASS::label_t label;
    parse_label(lp, &p, sd, "2 2", label);
    BOOST_ASSERT(label.label == 2);
    BOOST_ASSERT(label.weight == 2.0);
  }

  free(sd);
}