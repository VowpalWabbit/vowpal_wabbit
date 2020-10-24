#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "slates_label.h"
#include "parser.h"

void parse_label(label_parser& lp, parser* p, VW::string_view label, VW::slates::label& l)
{
  tokenize(' ', label, p->words);
  lp.default_label(&l);
  lp.parse_label(p, nullptr, &l, p->words);
}

BOOST_AUTO_TEST_CASE(slates_parse_label)
{
  auto lp = VW::slates::slates_label_parser;
  parser p{8 /*ring_size*/, false /*strict parse*/};

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    parse_label(lp, &p, "slates shared", *label);
    BOOST_CHECK_EQUAL(label->type, VW::slates::example_type::shared);
    BOOST_CHECK_CLOSE(label->cost, 0.f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->labeled, false);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    parse_label(lp, &p, "slates shared 1", *label);
    BOOST_CHECK_EQUAL(label->type, VW::slates::example_type::shared);
    BOOST_CHECK_CLOSE(label->cost, 1.f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label->labeled, true);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    parse_label(lp, &p, "slates action 1", *label);
    BOOST_CHECK_EQUAL(label->type, VW::slates::example_type::action);
    BOOST_CHECK_EQUAL(label->slot_id, 1);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    parse_label(lp, &p, "slates slot", *label);
    BOOST_CHECK_EQUAL(label->type, VW::slates::example_type::slot);
    BOOST_CHECK_EQUAL(label->labeled, false);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    parse_label(lp, &p, "slates slot 0:0.2", *label);
    BOOST_CHECK_EQUAL(label->type, VW::slates::example_type::slot);
    BOOST_CHECK_EQUAL(label->labeled, true);
    check_collections_with_float_tolerance(label->probabilities, std::vector<ACTION_SCORE::action_score>{{0,0.2f}}, FLOAT_TOL);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    parse_label(lp, &p, "slates slot 0:0.5,1:0.3,2:0.2", *label);
    BOOST_CHECK_EQUAL(label->type, VW::slates::example_type::slot);
    BOOST_CHECK_EQUAL(label->labeled, true);
    check_collections_with_float_tolerance(
        label->probabilities, std::vector<ACTION_SCORE::action_score>{{0, 0.5f}, {1, 0.3f}, {2, 0.2f}}, FLOAT_TOL);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "shared", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "slates shared 0.1 too many args", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "slates shared 0.1 too many args", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "slates action", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "slates action 1,1", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }

  {
    auto label = scoped_calloc_or_throw<VW::slates::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "slates slot 0:0,1:0.5", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }
}

BOOST_AUTO_TEST_CASE(slates_cache_shared_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  parser p{8 /*ring_size*/, false /*strict parse*/};

  auto lp = VW::slates::slates_label_parser;
  auto label = scoped_calloc_or_throw<VW::slates::label>();
  parse_label(lp, &p, "slates shared 0.5", *label.get());
  lp.cache_label(label.get(), io_writer);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  auto uncached_label = scoped_calloc_or_throw<VW::slates::label>();
  lp.default_label(uncached_label.get());
  lp.read_cached_label(nullptr, uncached_label.get(), io_reader);

  BOOST_CHECK_EQUAL(uncached_label->type, VW::slates::example_type::shared);
  BOOST_CHECK_EQUAL(uncached_label->labeled, true);
  BOOST_CHECK_CLOSE(uncached_label->cost, 0.5, FLOAT_TOL);
  lp.delete_label(label.get());
  lp.delete_label(uncached_label.get());}

BOOST_AUTO_TEST_CASE(slates_cache_action_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  parser p{8 /*ring_size*/, false /*strict parse*/};

  auto lp = VW::slates::slates_label_parser;
  auto label = scoped_calloc_or_throw<VW::slates::label>();
  parse_label(lp, &p, "slates action 5", *label.get());
  lp.cache_label(label.get(), io_writer);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  auto uncached_label = scoped_calloc_or_throw<VW::slates::label>();
  lp.default_label(uncached_label.get());
  lp.read_cached_label(nullptr, uncached_label.get(), io_reader);

  BOOST_CHECK_EQUAL(uncached_label->type, VW::slates::example_type::action);
  BOOST_CHECK_EQUAL(uncached_label->labeled, false);
  BOOST_CHECK_EQUAL(uncached_label->slot_id, 5);
  lp.delete_label(label.get());
  lp.delete_label(uncached_label.get());}


BOOST_AUTO_TEST_CASE(slates_cache_slot_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  parser p{8 /*ring_size*/, false /*strict parse*/};

  auto lp = VW::slates::slates_label_parser;
  auto label = scoped_calloc_or_throw<VW::slates::label>();
  parse_label(lp, &p, "slates slot 0:0.5,1:0.25,2:0.25", *label.get());
  lp.cache_label(label.get(), io_writer);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  auto uncached_label = scoped_calloc_or_throw<VW::slates::label>();
  lp.default_label(uncached_label.get());
  lp.read_cached_label(nullptr, uncached_label.get(), io_reader);

  BOOST_CHECK_EQUAL(uncached_label->type, VW::slates::example_type::slot);
  BOOST_CHECK_EQUAL(uncached_label->labeled, true);
  check_collections_with_float_tolerance(uncached_label->probabilities,
      std::vector<ACTION_SCORE::action_score>{{0, 0.5}, {1, 0.25}, {2, 0.25}}, FLOAT_TOL);
  lp.delete_label(label.get());
  lp.delete_label(uncached_label.get());}


BOOST_AUTO_TEST_CASE(slates_copy_label)
{
  parser p{8 /*ring_size*/, false /*strict parse*/};
  auto lp = VW::slates::slates_label_parser;

  auto label = scoped_calloc_or_throw<VW::slates::label>();
  parse_label(lp, &p, "slates slot 0:0.5,1:0.25,2:0.25", *label.get());

  auto copied_to = scoped_calloc_or_throw<VW::slates::label>();
  lp.default_label(copied_to.get());

  lp.copy_label(copied_to.get(), label.get());

  BOOST_CHECK_EQUAL(copied_to->type, VW::slates::example_type::slot);
  BOOST_CHECK_EQUAL(copied_to->labeled, true);
  check_collections_with_float_tolerance(
      copied_to->probabilities, std::vector<ACTION_SCORE::action_score>{{0, 0.5}, {1, 0.25}, {2, 0.25}}, FLOAT_TOL);
  lp.delete_label(label.get());
  lp.delete_label(copied_to.get());}
