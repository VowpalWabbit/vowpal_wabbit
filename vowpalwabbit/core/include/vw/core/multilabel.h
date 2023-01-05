// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/label_parser.h"
#include "vw/core/multi_ex.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

namespace MULTILABEL
{
class labels
{
public:
  VW::v_array<uint32_t> label_v;
};

void update_stats(const VW::workspace& all, const VW::example& ec);
void output_example_prediction(VW::workspace& all, const VW::example& ec);
void print_update(VW::workspace& all, const VW::example& ec);

extern VW::label_parser multilabel;

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
