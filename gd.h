/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef GD_H
#define GD_H

#include <math.h>
#include "example.h"
#include "parse_regressor.h"
#include "parser.h"

void print_result(int f, float res, v_array<char> tag);

struct gd_vars
{ 
  float eta;//learning rate control.
  float power_t;

  gd_vars()
  {};

  void init()
  {
    eta = 0.1;
    power_t = 0.;
  }
};

struct gd_thread_params
{
  gd_vars* vars;
  size_t thread_num;
  regressor reg;
  string* final_regressor_name;
};

float final_prediction(float ret, size_t num_features, float &norm);

float predict(weight* weights, const v_array<feature> &features);
float predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars);
float offset_predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars, size_t offset);

float inline_predict(regressor &reg, example* &ec, size_t thread_num);

float one_of_quad_predict(v_array<feature> &page_features, feature& offer_feature, weight* weights, size_t mask);

float one_pf_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask);

float single_quad_weight(weight* weights, feature& page_feature, feature* offer_feature, size_t mask);

void inline_train(regressor &reg, example* &ec, size_t thread_num, float update);

void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t thread_mask);

void train(weight* weights, const v_array<feature> &features, float update);

void train_one_example(regressor& r, example* ex, size_t thread_num, gd_vars& vars);
void train_offset_example(regressor& r, example* ex, size_t thread_num, gd_vars& vars, size_t offset);
void compute_update(example* ec, gd_vars& vars);
void offset_train(regressor &reg, example* &ec, size_t thread_num, float update, size_t offset);
void train_one_example_single_thread(regressor& r, example* ex, gd_vars& vars);
void setup_gd(gd_thread_params t);
void destroy_gd();
void output_and_account_example(example* ec);
void finish_example(example* ec);

#endif
