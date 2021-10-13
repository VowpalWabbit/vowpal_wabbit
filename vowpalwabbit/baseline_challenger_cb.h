// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "reductions_fwd.h"

struct io_buf;

VW::LEARNER::base_learner* baseline_challenger_cb_setup(VW::setup_base_i&);
namespace VW
{
struct discounted_expectation;
struct baseline_challenger_data;
namespace model_utils
{
size_t read_model_field(io_buf&, VW::discounted_expectation&);
size_t write_model_field(io_buf&, const VW::discounted_expectation&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::baseline_challenger_data&);
size_t write_model_field(io_buf&, const VW::baseline_challenger_data&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW