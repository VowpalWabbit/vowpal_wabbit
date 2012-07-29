/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PR_H
#define PR_H

#include <string>
#include "loss_functions.h"
#include "boost/program_options.hpp"
#include "global_data.h"

namespace po = boost::program_options;

using namespace std;

void parse_regressor_args(vw& all, po::variables_map& vm, string& final_regressor_name, bool quiet, bool initial_regressor_force_cubic_version);

void finalize_regressor(vw& all, std::string reg_name);
void save_predictor(vw& all, std::string reg_name, size_t current_pass);

#endif
