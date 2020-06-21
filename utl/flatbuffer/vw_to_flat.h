#pragma once

#include "../../vowpalwabbit/vw.h"
#include "../../vowpalwabbit/parser/flatbuffer/generated/example_generated.h"
#include "../../vowpalwabbit/simple_label.h"
#include "flatbuffers/flatbuffers.h"

struct flatbuilder
{
  flatbuffers::FlatBufferBuilder _builder;
  flatbuilder() : _builder(1) { }
};

void convert_txt_to_flat(vw& all);
void create_simple_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_cb_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_ccb_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_cb_eval_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_mc_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_multi_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_slates_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_cs_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
void create_no_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type);
