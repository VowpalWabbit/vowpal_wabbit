/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PR_H
#define PR_H
#include "loss_functions.h"
#include "boost/program_options.hpp"

namespace po = boost::program_options;

typedef float weight;

struct regressor {
  weight** weight_vectors;
  weight** regularizers;

  loss_function *loss;
};

void parse_regressor_args(po::variables_map& vm, regressor& r, std::string& final_regressor_name, bool quiet);

void initialize_regressor(regressor &r);

void finalize_regressor(std::string reg_name, regressor &r);
void dump_regressor(std::string reg_name, regressor &r, bool as_text=0, bool reg_vector=0);
void save_predictor(std::string reg_name, size_t current_pass);

#endif
