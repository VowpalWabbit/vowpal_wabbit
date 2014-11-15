/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <string>
#include "boost/program_options.hpp"
#include "global_data.h"

namespace po = boost::program_options;

using namespace std;

void parse_regressor_args(vw& all, po::variables_map& vm, io_buf& io_temp);

void finalize_regressor(vw& all, std::string reg_name);
void initialize_regressor(vw& all);

void save_predictor(vw& all, std::string reg_name, size_t current_pass);
void save_load_header(vw& all, io_buf& model_file, bool read, bool text);

void parse_mask_regressor_args(vw& all, po::variables_map& vm);
