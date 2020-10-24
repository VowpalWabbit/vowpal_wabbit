#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "contcb_label.h"
#include "parser.h"

void parse_label(label_parser& lp, parser* p, VW::string_view label, VW::continuous_cb::label& l)
{
  tokenize(' ', label, p->words);
  lp.default_label(&l);
  lp.parse_label(p, nullptr, &l, p->words);
}

BOOST_AUTO_TEST_CASE(contcb_parse_label)
{
  auto lp = VW::continuous_cb::contcb_label;
  parser p{8 /*ring_size*/, false /*strict parse*/};

  // standard case
  {
    auto label = scoped_calloc_or_throw<VW::continuous_cb::label>();
    parse_label(lp, &p, "-25.829122:-9.45", *label);
    BOOST_CHECK_CLOSE(label->action, -25.829122f, FLOAT_TOL);
    BOOST_CHECK_CLOSE(label->cost, -9.45f, FLOAT_TOL);
    lp.delete_label(label.get());
  }

  // colon must be the separater for action and cost
  {
    auto label = scoped_calloc_or_throw<VW::continuous_cb::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "-25.82912 -9.45", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }

  // both action and cost must be supplied
  {
    auto label = scoped_calloc_or_throw<VW::continuous_cb::label>();
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "-25.82912", *label.get()), VW::vw_exception);
    lp.delete_label(label.get());
  }
}

BOOST_AUTO_TEST_CASE(contcb_cache_label)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  parser p{8 /*ring_size*/, false /*strict parse*/};

  auto lp = VW::continuous_cb::contcb_label;
  auto label = scoped_calloc_or_throw<VW::continuous_cb::label>();
  parse_label(lp, &p, "-25.829122:-9.45", *label.get());
  lp.cache_label(label.get(), io_writer);
  io_writer.flush();

  io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  auto uncached_label = scoped_calloc_or_throw<VW::continuous_cb::label>();
  lp.default_label(uncached_label.get());
  lp.read_cached_label(nullptr, uncached_label.get(), io_reader);

  BOOST_CHECK_CLOSE(label->action, -25.829122f, FLOAT_TOL);
  BOOST_CHECK_CLOSE(label->cost, -9.45f, FLOAT_TOL);
  lp.delete_label(label.get());
  lp.delete_label(uncached_label.get());
}
