// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "io/logger.h"
#include "memory.h"
#include "test_common.h"

#include <vector>
#include "reductions/conditional_contextual_bandit.h"
#include "parser.h"
#include "parse_primitives.h"
#include "vowpalwabbit/vw_string_view.h"
#include <memory>

void parse_ccb_label(VW::string_view label, CCB::label& l)
{
  std::vector<VW::string_view> words;
  tokenize(' ', label, words);
  CCB::default_label(l);
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  CCB::parse_label(l, mem, words, null_logger);
}

BOOST_AUTO_TEST_CASE(ccb_parse_label)
{
  {
    auto label = VW::make_unique<CCB::label>();
    parse_ccb_label("ccb shared", *label);
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::shared);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    parse_ccb_label("ccb action", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::action);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    parse_ccb_label("ccb slot", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 0);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    parse_ccb_label("ccb slot 1,3,4", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 3);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[0], 1);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[1], 3);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[2], 4);
    BOOST_CHECK(label->outcome == nullptr);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    parse_ccb_label("ccb slot 1:1.0:0.5 3", *label.get());
    BOOST_CHECK_EQUAL(label->explicit_included_actions.size(), 1);
    BOOST_CHECK_EQUAL(label->explicit_included_actions[0], 3);
    BOOST_CHECK_CLOSE(label->outcome->cost, 1.0f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->outcome->probabilities.size(), 1);
    BOOST_CHECK_EQUAL(label->outcome->probabilities[0].action, 1);
    BOOST_CHECK_CLOSE(label->outcome->probabilities[0].score, .5f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->type, CCB::example_type::slot);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    parse_ccb_label("ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());
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
  }
  {
    auto label = VW::make_unique<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label("shared", *label.get()), VW::vw_exception);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label("other shared", *label.get()), VW::vw_exception);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label("other", *label.get()), VW::vw_exception);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label("ccb unknown", *label.get()), VW::vw_exception);
  }
  {
    auto label = VW::make_unique<CCB::label>();
    BOOST_REQUIRE_THROW(parse_ccb_label("ccb slot 1:1.0:0.5,4:0.7", *label.get()), VW::vw_exception);
  }
}

BOOST_AUTO_TEST_CASE(ccb_cache_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  auto label = VW::make_unique<CCB::label>();
  parse_ccb_label("ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());
  VW::model_utils::write_model_field(io_writer, *label, "", false);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  auto uncached_label = VW::make_unique<CCB::label>();
  CCB::default_label(*uncached_label);
  VW::model_utils::read_model_field(io_reader, *uncached_label);

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
}

BOOST_AUTO_TEST_CASE(ccb_copy_label)
{
  auto label = VW::make_unique<CCB::label>();
  parse_ccb_label("ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", *label.get());

  auto copied_to = VW::make_unique<CCB::label>();
  CCB::default_label(*copied_to);

  *copied_to = *label;
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
}
