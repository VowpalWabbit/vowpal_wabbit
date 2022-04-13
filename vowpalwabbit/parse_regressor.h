// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "vw/config/options.h"
#include "vw_fwd.h"

#include <string>

void read_regressor_file(VW::workspace& all, const std::vector<std::string>& files, io_buf& io_temp);

void finalize_regressor(VW::workspace& all, const std::string& reg_name);
void initialize_regressor(VW::workspace& all);

void save_predictor(VW::workspace& all, const std::string& reg_name, size_t current_pass);
void save_load_header(VW::workspace& all, io_buf& model_file, bool read, bool text, std::string& file_options,
    VW::config::options_i& options);

void parse_mask_regressor_args(
    VW::workspace& all, const std::string& feature_mask, std::vector<std::string> initial_regressors);
