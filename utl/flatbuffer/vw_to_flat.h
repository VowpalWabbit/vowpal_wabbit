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

class to_flat
{
public:
  std::string output_flatbuffer_name;
  void convert_txt_to_flat(vw& all);

private:
  flatbuffers::FlatBufferBuilder _builder;
  void create_simple_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
  void create_cb_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
  void create_ccb_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
  void create_cb_eval_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
  void create_mc_label(VW::named_labels* ldict, example* v, flatbuffers::Offset<void>& label,
      VW::parsers::flatbuffer::Label& label_type);
  void create_multi_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
  void create_slates_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
  void create_cs_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
  void create_no_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
};
