// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "io_buf.h"
#include "label_parser.h"
#include "v_array.h"

namespace VW
{
struct workspace;
struct example;
}  // namespace VW

namespace MULTILABEL
{
struct labels
{
  VW::v_array<uint32_t> label_v;
};

void output_example(VW::workspace& all, const VW::example& ec);

extern VW::label_parser multilabel;

void print_update(VW::workspace& all, bool is_test, const VW::example& ec, const VW::v_array<VW::example*>* ec_seq);
}  // namespace MULTILABEL

namespace VW
{
std::string to_string(const MULTILABEL::labels& multilabels);

namespace model_utils
{
size_t read_model_field(io_buf&, MULTILABEL::labels&);
size_t write_model_field(io_buf&, const MULTILABEL::labels&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
