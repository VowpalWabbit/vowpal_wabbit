// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "io/logger.h"
#include "test_common.h"

#include <vector>
#include "slates_label.h"
#include "parser.h"
#include "parse_primitives.h"
#include "vw_string_view.h"

void parse_slates_label(VW::string_view label, VW::slates::label& l)
{
  std::vector<VW::string_view> words;
  tokenize(' ', label, words);
  VW::slates::default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(l, mem, words, null_logger);
}

BOOST_AUTO_TEST_CASE(slates_parse_label)
{
  {
    VW::slates::label label;
    parse_slates_label("slates shared", label);
    BOOST_CHECK_EQUAL(label.type, VW::slates::example_type::shared);
    BOOST_CHECK_CLOSE(label.cost, 0.f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label.labeled, false);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates shared 1", label);
    BOOST_CHECK_EQUAL(label.type, VW::slates::example_type::shared);
    BOOST_CHECK_CLOSE(label.cost, 1.f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label.labeled, true);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates action 1", label);
    BOOST_CHECK_EQUAL(label.type, VW::slates::example_type::action);
    BOOST_CHECK_EQUAL(label.slot_id, 1);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates slot", label);
    BOOST_CHECK_EQUAL(label.type, VW::slates::example_type::slot);
    BOOST_CHECK_EQUAL(label.labeled, false);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates slot 0:0.2", label);
    BOOST_CHECK_EQUAL(label.type, VW::slates::example_type::slot);
    BOOST_CHECK_EQUAL(label.labeled, true);
    check_collections_with_float_tolerance(
        label.probabilities, std::vector<ACTION_SCORE::action_score>{{0, 0.2f}}, FLOAT_TOL);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates slot 0:0.5,1:0.3,2:0.2", label);
    BOOST_CHECK_EQUAL(label.type, VW::slates::example_type::slot);
    BOOST_CHECK_EQUAL(label.labeled, true);
    check_collections_with_float_tolerance(
        label.probabilities, std::vector<ACTION_SCORE::action_score>{{0, 0.5f}, {1, 0.3f}, {2, 0.2f}}, FLOAT_TOL);
  }

  {
    VW::slates::label label;
    BOOST_REQUIRE_THROW(parse_slates_label("shared", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    BOOST_REQUIRE_THROW(parse_slates_label("slates shared 0.1 too many args", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    BOOST_REQUIRE_THROW(parse_slates_label("slates shared 0.1 too many args", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    BOOST_REQUIRE_THROW(parse_slates_label("slates action", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    BOOST_REQUIRE_THROW(parse_slates_label("slates action 1,1", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    BOOST_REQUIRE_THROW(parse_slates_label("slates slot 0:0,1:0.5", label), VW::vw_exception);
  }
}

BOOST_AUTO_TEST_CASE(slates_cache_shared_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::slates::label label;
  parse_slates_label("slates shared 0.5", label);
  VW::model_utils::write_model_field(io_writer, label, "", false);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::slates::label uncached_label;
  VW::slates::default_label(uncached_label);
  VW::model_utils::read_model_field(io_reader, uncached_label);

  BOOST_CHECK_EQUAL(uncached_label.type, VW::slates::example_type::shared);
  BOOST_CHECK_EQUAL(uncached_label.labeled, true);
  BOOST_CHECK_CLOSE(uncached_label.cost, 0.5, FLOAT_TOL);
}

BOOST_AUTO_TEST_CASE(slates_cache_action_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::slates::label label;
  parse_slates_label("slates action 5", label);
  VW::model_utils::write_model_field(io_writer, label, "", false);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::slates::label uncached_label;
  VW::slates::default_label(uncached_label);
  VW::model_utils::read_model_field(io_reader, uncached_label);

  BOOST_CHECK_EQUAL(uncached_label.type, VW::slates::example_type::action);
  BOOST_CHECK_EQUAL(uncached_label.labeled, false);
  BOOST_CHECK_EQUAL(uncached_label.slot_id, 5);
}

BOOST_AUTO_TEST_CASE(slates_cache_slot_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::slates::label label;
  parse_slates_label("slates slot 0:0.5,1:0.25,2:0.25", label);
  VW::model_utils::write_model_field(io_writer, label, "", false);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::slates::label uncached_label;
  VW::slates::default_label(uncached_label);
  VW::model_utils::read_model_field(io_reader, uncached_label);

  BOOST_CHECK_EQUAL(uncached_label.type, VW::slates::example_type::slot);
  BOOST_CHECK_EQUAL(uncached_label.labeled, true);
  check_collections_with_float_tolerance(
      uncached_label.probabilities, std::vector<ACTION_SCORE::action_score>{{0, 0.5}, {1, 0.25}, {2, 0.25}}, FLOAT_TOL);
}

BOOST_AUTO_TEST_CASE(slates_copy_label)
{
  VW::slates::label label;
  parse_slates_label("slates slot 0:0.5,1:0.25,2:0.25", label);

  VW::slates::label copied_to;
  VW::slates::default_label(copied_to);
  copied_to = label;
  BOOST_CHECK_EQUAL(copied_to.type, VW::slates::example_type::slot);
  BOOST_CHECK_EQUAL(copied_to.labeled, true);
  check_collections_with_float_tolerance(
      copied_to.probabilities, std::vector<ACTION_SCORE::action_score>{{0, 0.5}, {1, 0.25}, {2, 0.25}}, FLOAT_TOL);
}
