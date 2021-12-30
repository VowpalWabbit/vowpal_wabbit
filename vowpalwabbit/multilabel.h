// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"
#include "v_array.h"
#include "io_buf.h"

struct example;
namespace VW
{
struct workspace;
}

namespace MULTILABEL
{
struct labels
{
  v_array<uint32_t> label_v;
};

void output_example(VW::workspace& all, const example& ec);

extern label_parser multilabel;

void print_update(VW::workspace& all, bool is_test, const example& ec, const v_array<example*>* ec_seq);
}  // namespace MULTILABEL

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, MULTILABEL::labels&);
size_t write_model_field(io_buf&, const MULTILABEL::labels&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
