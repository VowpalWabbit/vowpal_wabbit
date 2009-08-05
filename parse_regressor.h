/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PR_H
#define PR_H
#include "static_data.h"
#include "loss_functions.h"
#include "boost/program_options.hpp"

namespace po = boost::program_options;

typedef float weight;

struct regressor {
  weight** weight_vectors;

  static_data* global;

  loss_function *loss;
};

void initialize_regressor(regressor &r);
void parse_regressor(vector<string> regs, regressor &r);

void finalize_regressor(ofstream& o, regressor &r);
void dump_regressor(ofstream &o, regressor &r);

#endif
