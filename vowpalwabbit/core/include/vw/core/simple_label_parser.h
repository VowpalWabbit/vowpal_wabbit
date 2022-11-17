// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/label_parser.h"
#include "vw/core/simple_label.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
extern VW::label_parser simple_label_parser_global;

namespace model_utils
{
size_t read_model_field(io_buf&, simple_label&);
size_t write_model_field(io_buf&, const simple_label&, const std::string&, bool);
size_t read_model_field(io_buf&, simple_label_reduction_features&);
size_t write_model_field(io_buf&, const simple_label_reduction_features&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW