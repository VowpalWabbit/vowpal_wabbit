// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_continuous_label.h"
#include "parse_primitives.h"
#include "parser.h"
#include "test_common.h"
#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/io/logger.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include <vector>

void parse_label(VW::label_parser& lp, VW::string_view label, VW::polylabel& l, VW::reduction_features& red_fts)
{
  std::vector<VW::string_view> words;
  VW::common::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_label)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca 185.121:0.657567:6.20426e-05", *plabel, red_features);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].cost, 0.657567, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].action, 185.121, FLOAT_TOL);

    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_label_and_pdf)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;

    parse_label(lp, "ca 185.121:0.657567:6.20426e-05 pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05", *plabel,
        red_features);
    // check label
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].cost, 0.657567, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].action, 185.121, FLOAT_TOL);

    // check pdf
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 2);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].left, 185., FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].right, 8109.67, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].left, 8109.67, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].right, 23959., FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05, FLOAT_TOL);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_only_pdf_no_label)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05", *plabel, red_features);
    BOOST_CHECK_EQUAL(plabel->cb_cont.costs.size(), 0);

    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 2);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].left, 185., FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].right, 8109.67, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].left, 8109.67, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].right, 23959., FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05, FLOAT_TOL);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_malformed_pdf)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;

    parse_label(lp, "ca pdf 185:8109.67 8109.67:23959:6.20426e-05", *plabel, red_features);

    // check pdf
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 0);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_label_and_chosen_action)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca 185.121:0.657567:6.20426e-05 chosen_action 8110.121", *plabel, red_features);

    // check label
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].cost, 0.657567, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].action, 185.121, FLOAT_TOL);

    // check chosen action
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), true);
    BOOST_CHECK_CLOSE(cats_reduction_features.chosen_action, 8110.121, FLOAT_TOL);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_chosen_action_only_no_label)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca chosen_action 8110.121", *plabel, red_features);

    BOOST_CHECK_EQUAL(plabel->cb_cont.costs.size(), 0);
    // check chosen action
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), true);
    BOOST_CHECK_CLOSE(cats_reduction_features.chosen_action, 8110.121, FLOAT_TOL);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_label_pdf_and_chosen_action)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp,
        "ca 185.121:0.657567:6.20426e-05 pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05 chosen_action 8110.121",
        *plabel, red_features);

    // check label
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].cost, 0.657567, FLOAT_TOL);
    BOOST_CHECK_CLOSE(plabel->cb_cont.costs[0].action, 185.121, FLOAT_TOL);

    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();

    // check chosen action
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), true);
    BOOST_CHECK_CLOSE(cats_reduction_features.chosen_action, 8110.121, FLOAT_TOL);

    // check pdf
    BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 2);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].left, 185., FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].right, 8109.67, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].left, 8109.67, FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].right, 23959., FLOAT_TOL);
    BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05, FLOAT_TOL);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_no_label)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "", *plabel, red_features);

    BOOST_CHECK_EQUAL(plabel->cb_cont.costs.size(), 0);
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
  }
}

BOOST_AUTO_TEST_CASE(continuous_actions_parse_no_label_w_prefix)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca", *plabel, red_features);

    BOOST_CHECK_EQUAL(plabel->cb_cont.costs.size(), 0);
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
    BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
  }
}

BOOST_AUTO_TEST_CASE(continus_actions_check_label_for_prefix)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = scoped_calloc_or_throw<VW::polylabel>();
    VW::reduction_features red_features;
    BOOST_REQUIRE_THROW(parse_label(lp, "185.121:0.657567:6.20426e-05", *plabel, red_features), VW::vw_exception);
  }
}
