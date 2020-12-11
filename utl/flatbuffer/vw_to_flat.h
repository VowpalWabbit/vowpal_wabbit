// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw.h"
VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_BADLY_FORMED_XML
#include "parser/flatbuffer/generated/example_generated.h"
VW_WARNING_STATE_POP
#include "simple_label.h"

struct MultiExampleBuilder
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  VW::parsers::flatbuffer::Label shared_type = VW::parsers::flatbuffer::Label_NONE;
  flatbuffers::Offset<void> shared;
  std::vector<uint8_t> label_types;
  std::vector<flatbuffers::Offset<void>> labels;
  uint32_t label_index = 0;
  bool label_index_set = false;
};

struct ExampleBuilder
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  VW::parsers::flatbuffer::Label label_type = VW::parsers::flatbuffer::Label_NONE;
  flatbuffers::Offset<void> label = 0;
  flatbuffers::Offset<flatbuffers::String> tag;
};

class to_flat
{
public:
  std::string output_flatbuffer_name;
  size_t collection_size = 0;
  bool collection = false;
  bool is_multiline = false;
  void convert_txt_to_flat(vw& all);

private:
  flatbuffers::FlatBufferBuilder _builder;
  void create_simple_label(example* v, ExampleBuilder& ex_builder);
  void create_cb_label(example* v, ExampleBuilder& ex_builder);
  void create_cb_label_multi_ex(example* v, MultiExampleBuilder& ex_builder);
  void create_ccb_label_multi_ex(example* v, MultiExampleBuilder& ex_builder);
  void create_cb_eval_label(example* v, ExampleBuilder& ex_builder);
  void create_mc_label(VW::named_labels* ldict, example* v, ExampleBuilder& ex_builder);
  void create_multi_label(example* v, ExampleBuilder& ex_builder);
  void create_slates_label(example* v, ExampleBuilder& ex_builder);
  void create_cs_label(example* v, ExampleBuilder& ex_builder);
  void create_no_label(example* v, ExampleBuilder& ex_builder);
};
