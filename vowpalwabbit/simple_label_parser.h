// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "label_parser.h"
#include "simple_label.h"

extern VW::label_parser simple_label_parser;

class io_buf;
namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, label_data&);
size_t write_model_field(io_buf&, const label_data&, const std::string&, bool);
size_t read_model_field(io_buf&, simple_label_reduction_features&);
size_t write_model_field(io_buf&, const simple_label_reduction_features&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW