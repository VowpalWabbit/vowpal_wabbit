#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "conditional_contextual_bandit.h"
#include "parser.h"

void parse_label(label_parser& lp, parser* p, VW::string_view label, CCB::label& l)
{
  tokenize(' ', label, p->words);
  lp.default_label(&l);
  lp.parse_label(p, nullptr, &l, p->words);
}

BOOST_AUTO_TEST_CASE(ccb_parse_label)
{
  auto lp = CCB::ccb_label_parser;
  parser p{8 /*ring_size*/, false /*strict parse*/};
  p.words = v_init<VW::string_view>();
  p.parse_name = v_init<VW::string_view>();

  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_label(lp, &p, "ccb shared", *label);
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::shared);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_label(lp, &p, "ccb action", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::action);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_label(lp, &p, "ccb slot", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_label(lp, &p, "ccb slot 1,3,4", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 3);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[0], 1);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[1], 3);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[2], 4);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_label(lp, &p, "ccb slot 1:1.0:0.5 3", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 1);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[0], 3);
    BOOST_CHECK_CLOSE(label->outcome->cost, 1.0f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->outcome->probabilities.size(), 1);
    BOOST_CHECK_EQUAL(label->outcome->probabilities[0].action, 1);
    BOOST_CHECK_CLOSE(label->outcome->probabilities[0].score, .5f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    parse_label(lp, &p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());
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
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "shared", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "other shared", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "other", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "ccb unknown", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }
  {
    auto label = scoped_calloc_or_throw<CCB::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "ccb slot 1:1.0:0.5,4:0.7", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
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
  parse_label(lp, &p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());
  lp.cache_label(label.get(), io);
  io.space.end() = io.head;
  io.head = io.space.begin();

  auto uncached_label = scoped_calloc_or_throw<CCB::label>();
  lp.default_label(uncached_label.get());
  lp.read_cached_label(nullptr, uncached_label.get(), io);

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
  lp.delete_label(label.get());
  lp.delete_label(uncached_label.get());
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
  parse_label(lp, &p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());

  auto copied_to = scoped_calloc_or_throw<CCB::label>();
  lp.default_label(copied_to.get());

  lp.copy_label(copied_to.get(), label.get());

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
  lp.delete_label(label.get());
  lp.delete_label(copied_to.get());
  p.words.delete_v();
  p.parse_name.delete_v();
}
