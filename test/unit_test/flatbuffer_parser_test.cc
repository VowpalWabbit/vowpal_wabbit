// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "parser/flatbuffer/parse_example_flatbuffer.h"
#include "constant.h"

multi_ex parse_flatbuffer(vw& all)
{
  auto examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(&all));

  VW::parsers::flatbuffer::flatbuffer_to_examples(&all, examples);

  multi_ex result;
  for (size_t i = 0; i < examples.size(); ++i) { result.push_back(examples[i]); }
  examples.delete_v();
  return result;
}

flatbuffers::Offset<void> get_label(flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  flatbuffers::Offset<void> label;
  if (label_type == VW::parsers::flatbuffer::Label_SimpleLabel)
    label = VW::parsers::flatbuffer::CreateSimpleLabel(builder, 0.0, 1.0).Union();

  return label;
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleCollection> sample_flatbuffer(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> examplecollection;
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;

  auto label = get_label(builder, label_type);

  fts.push_back(VW::parsers::flatbuffer::CreateFeatureDirect(builder, "hello", 2.23f, constant));
  namespaces.push_back(
      VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, constant_namespace, &fts));
  examplecollection.push_back(
      VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label));

  return VW::parsers::flatbuffer::CreateExampleCollectionDirect(builder, &examplecollection);
}

BOOST_AUTO_TEST_CASE(check_flatbuffer)
{
  // How to get rid of reading a file?
  auto all = VW::initialize("--no_stdin --quiet", nullptr, false, nullptr, nullptr);

  flatbuffers::FlatBufferBuilder builder;

  auto egcollection = sample_flatbuffer(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.Finish(egcollection);

  uint8_t* buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  all->flat_converter = VW::make_unique<VW::parsers::flatbuffer::parser>(buf);

  BOOST_CHECK_EQUAL(all->flat_converter->data()->examples()->Length(), 1);
  BOOST_CHECK_EQUAL(all->flat_converter->data()->examples()->Get(0)->namespaces()->Length(), 1);
  BOOST_CHECK_EQUAL(all->flat_converter->data()->examples()->Get(0)->namespaces()->Get(0)->features()->size(), 1);
  BOOST_CHECK_CLOSE(all->flat_converter->data()->examples()->Get(0)->label_as_SimpleLabel()->label(), 0.0, FLOAT_TOL);
  BOOST_CHECK_CLOSE(all->flat_converter->data()->examples()->Get(0)->label_as_SimpleLabel()->weight(), 1.0, FLOAT_TOL);
  BOOST_CHECK_EQUAL(all->flat_converter->data()->examples()->Get(0)->namespaces()->Get(0)->hash(), constant_namespace);
  BOOST_CHECK_EQUAL(
      all->flat_converter->data()->examples()->Get(0)->namespaces()->Get(0)->features()->Get(0)->name()->c_str(),
      "hello");
  BOOST_CHECK_EQUAL(
      all->flat_converter->data()->examples()->Get(0)->namespaces()->Get(0)->features()->Get(0)->hash(), constant);
  BOOST_CHECK_CLOSE(all->flat_converter->data()->examples()->Get(0)->namespaces()->Get(0)->features()->Get(0)->value(),
      2.23, FLOAT_TOL);
  VW::finish(*all);
}

BOOST_AUTO_TEST_CASE(check_parsed_flatbuffer_examples)
{
  auto all = VW::initialize("--no_stdin --quiet", nullptr, false, nullptr, nullptr);

  flatbuffers::FlatBufferBuilder builder;
  auto egcollection = sample_flatbuffer(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.Finish(egcollection);

  uint8_t* buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  all->flat_converter = VW::make_unique<VW::parsers::flatbuffer::parser>(buf);
  auto examples = parse_flatbuffer(*all);

  BOOST_CHECK_EQUAL(examples.size(), 1);
  BOOST_CHECK_CLOSE(examples[0]->l.simple.label, 0.f, FLOAT_TOL);
  BOOST_CHECK_CLOSE(examples[0]->l.simple.weight, 1.f, FLOAT_TOL);

  BOOST_CHECK_EQUAL(examples[0]->indices[0], constant_namespace);
  BOOST_CHECK_CLOSE(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f, FLOAT_TOL);
  VW::finish_example(*all, examples);
  VW::finish(*all);
}