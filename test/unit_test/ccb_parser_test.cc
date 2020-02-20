#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "conditional_contextual_bandit.h"
#include "parser.h"

void parse_ccb_label(parser* p, VW::string_view label, CCB::label& l)
{
  tokenize(' ', label, p->words);
  CCB::default_label(l);
  CCB::parse_label(p, nullptr, l, p->words);
}

BOOST_AUTO_TEST_CASE(ccb_parse_label)
{
  auto lp = CCB::ccb_label_parser;
  parser p{8 /*ring_size*/, false /*strict parse*/};
  p.words = v_init<VW::string_view>();
  p.parse_name = v_init<VW::string_view>();

  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_ccb_label(&p, "ccb shared", *label);
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::shared);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_ccb_label(&p, "ccb action", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::action);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_ccb_label(&p, "ccb slot", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_ccb_label(&p, "ccb slot 1,3,4", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 3);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[0], 1);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[1], 3);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[2], 4);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_ccb_label(&p, "ccb slot 1:1.0:0.5 3", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 1);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[0], 3);
    BOOST_CHECK_CLOSE(label->outcome->cost, 1.0f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->outcome->probabilities.size(), 1);
    BOOST_CHECK_EQUAL(label->outcome->probabilities[0].action, 1);
    BOOST_CHECK_CLOSE(label->outcome->probabilities[0].score, .5f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_ccb_label(&p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 2);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[0], 3);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[1], 4);
    BOOST_CHECK_CLOSE(label->outcome->cost, -2.0f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->outcome->probabilities.size(), 3);
    BOOST_CHECK_EQUAL(label->outcome->probabilities[0].action, 1);
    BOOST_CHECK_CLOSE(label->outcome->probabilities[0].score, .5f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->outcome->probabilities[1].action, 2);
    BOOST_CHECK_CLOSE(label->outcome->probabilities[1].score, .25f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->outcome->probabilities[2].action, 3);
    BOOST_CHECK_CLOSE(label->outcome->probabilities[2].score, .25f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label(&p, "shared", *label.get()), VW::vw_exception);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label(&p, "other shared", *label.get()), VW::vw_exception);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label(&p, "other", *label.get()), VW::vw_exception);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label(&p, "ccb unknown", *label.get()), VW::vw_exception);
    CCB::delete_label(*label);
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label(&p, "ccb slot 1:1.0:0.5,4:0.7", *label.get()), VW::vw_exception);
    CCB::delete_label(*label);
  }
  p.words.delete_v();
  p.parse_name.delete_v();
}

BOOST_AUTO_TEST_CASE(ccb_cache_label)
{
  io_buf io;
  //io.init();      TODO: figure out and fix leak caused by double init()

  parser p{8 /*ring_size*/, false /*strict parse*/};
  p.words = v_init<VW::string_view>();
  p.parse_name = v_init<VW::string_view>();

  auto lp = CCB::ccb_label_parser;
  auto label = scoped_calloc_or_throw<CCB::label>();
  parse_ccb_label(&p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());
  CCB::cache_label(*label, io);
  io.space.end() = io.head;
  io.head = io.space.begin();

  auto uncached_label = scoped_calloc_or_throw<CCB::label>();
  CCB::default_label(*uncached_label);
  CCB::read_cached_label(nullptr, *uncached_label, io);

  BOOST_CHECK_EQUAL(uncached_label->explicit_included_actions.size(), 2);
  BOOST_CHECK_EQUAL(uncached_label->explicit_included_actions[0], 3);
  BOOST_CHECK_EQUAL(uncached_label->explicit_included_actions[1], 4);
  BOOST_CHECK_CLOSE(uncached_label->outcome->cost, -2.0f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label->outcome->probabilities.size(), 3);
  BOOST_CHECK_EQUAL(uncached_label->outcome->probabilities[0].action, 1);
  BOOST_CHECK_CLOSE(uncached_label->outcome->probabilities[0].score, .5f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label->outcome->probabilities[1].action, 2);
  BOOST_CHECK_CLOSE(uncached_label->outcome->probabilities[1].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label->outcome->probabilities[2].action, 3);
  BOOST_CHECK_CLOSE(uncached_label->outcome->probabilities[2].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label->type, CCB::example_type::slot);
  CCB::delete_label(*label);
  CCB::delete_label(*uncached_label);
  p.words.delete_v();
  p.parse_name.delete_v();
}

BOOST_AUTO_TEST_CASE(ccb_copy_label)
{
  parser p{8 /*ring_size*/, false /*strict parse*/};
  p.words = v_init<VW::string_view>();
  p.parse_name = v_init<VW::string_view>();
  auto lp = CCB::ccb_label_parser;

  auto label = scoped_calloc_or_throw<CCB::label>();
  parse_ccb_label(&p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());

  auto copied_to = scoped_calloc_or_throw<CCB::label>();
  CCB::default_label(*copied_to);

  CCB::copy_label(*copied_to, *label);

  BOOST_CHECK_EQUAL(copied_to->explicit_included_actions.size(), 2);
  BOOST_CHECK_EQUAL(copied_to->explicit_included_actions[0], 3);
  BOOST_CHECK_EQUAL(copied_to->explicit_included_actions[1], 4);
  BOOST_CHECK_CLOSE(copied_to->outcome->cost, -2.0f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to->outcome->probabilities.size(), 3);
  BOOST_CHECK_EQUAL(copied_to->outcome->probabilities[0].action, 1);
  BOOST_CHECK_CLOSE(copied_to->outcome->probabilities[0].score, .5f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to->outcome->probabilities[1].action, 2);
  BOOST_CHECK_CLOSE(copied_to->outcome->probabilities[1].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to->outcome->probabilities[2].action, 3);
  BOOST_CHECK_CLOSE(copied_to->outcome->probabilities[2].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to->type, CCB::example_type::slot);
  CCB::delete_label(*label);
  CCB::delete_label(*copied_to);
  p.words.delete_v();
  p.parse_name.delete_v();
}
