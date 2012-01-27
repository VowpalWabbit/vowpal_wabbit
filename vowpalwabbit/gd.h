/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef GD_H
#define GD_H

#ifdef __FreeBSD__
#include <sys/socket.h>
#endif

#include <math.h>
#include "example.h"
#include "parse_regressor.h"
#include "parser.h"
#include "allreduce.h"

void print_result(int f, float res, v_array<char> tag);

void print_audit_features(regressor &reg, example* ec, size_t offset);

float finalize_prediction(float ret);

void predict(regressor& r, example* ex);

float inline_predict(regressor &reg, example* &ec);

float one_of_quad_predict(v_array<feature> &page_features, feature& offer_feature, weight* weights, size_t mask);

float one_pf_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask);

float single_quad_weight(weight* weights, feature& page_feature, feature* offer_feature, size_t mask);

void inline_train(regressor &reg, example* &ec, float update);

void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t thread_mask);

void print_audit_features(regressor &reg, example* ec);

void train(weight* weights, const v_array<feature> &features, float update);

void train_one_example(regressor& r, example* ex);
void train_offset_example(regressor& r, example* ex, size_t offset);
void compute_update(example* ec);
void offset_train(regressor &reg, example* &ec, float update, size_t offset);
void train_one_example_single_thread(regressor& r, example* ex);
void drive_gd();
void finish_gd();
void learn_gd(example* ec);

void output_and_account_example(example* ec);
void finish_example(example* ec);

bool command_example(example* ec);

void sync_weights(regressor *reg);

#endif
