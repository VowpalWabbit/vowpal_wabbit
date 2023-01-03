// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "vw/config/options.h"
#include "vw/core/vw_fwd.h"

#include <string>

namespace VW
{
namespace details
{
void read_regressor_file(VW::workspace& all, const std::vector<std::string>& files, VW::io_buf& io_temp);

void finalize_regressor(VW::workspace& all, const std::string& reg_name);
void initialize_regressor(VW::workspace& all);

void save_predictor(VW::workspace& all, const std::string& reg_name, size_t current_pass);
void save_load_header(VW::workspace& all, VW::io_buf& model_file, bool read, bool text, std::string& file_options,
    VW::config::options_i& options);

void parse_mask_regressor_args(
    VW::workspace& all, const std::string& feature_mask, std::vector<std::string> initial_regressors);

void dump_regressor(VW::workspace& all, io_buf& buf, bool as_text);
void dump_regressor(VW::workspace& all, const std::string& reg_name, bool as_text);
}  // namespace details
}  // namespace VW
