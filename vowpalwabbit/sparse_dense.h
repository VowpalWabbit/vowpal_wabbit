/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SPARSE_DENSE_VECTOR_H
#define SPARSE_DENSE_VECTOR_H

#include <math.h>
#include "parse_example.h"

inline float sign(float w){ if (w < 0.) return -1.; else  return 1.;}

inline float trunc_weight(float w, float gravity){
  return (gravity < fabsf(w)) ? w - sign(w) * gravity : 0.f;
}

float sd_add(weight* weights, size_t mask, feature* begin, feature* end);
float sd_add_trunc(weight* weights, size_t mask, feature* begin, feature* end, float gravity);
float sd_add_rescale(weight* weights, size_t mask, feature* begin, feature* end, bool is_adaptive, size_t idx_norm);
float sd_add_trunc_rescale(weight* weights, size_t mask, feature* begin, feature* end, float gravity, bool is_adaptive, size_t idx_norm);
float sd_add_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, size_t idx_norm, float power_t_norm);
float sd_add_trunc_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, float gravity, size_t idx_norm, float power_t_norm);

float sd_offset_add(weight* weights, size_t mask, feature* begin, feature* end, size_t offset);
void sd_offset_update(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float update, float regularization);

void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t thread_mask);

float one_of_quad_predict(v_array<feature> &page_features, feature& offer_feature, weight* weights, size_t mask);

float one_pf_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask);

float one_pf_quad_predict_trunc(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, float gravity);

float one_pf_quad_predict_rescale(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, bool is_adaptive, size_t idx_norm);
float one_pf_quad_predict_trunc_rescale(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, float gravity, bool is_adaptive, size_t idx_norm);
float one_pf_quad_predict_rescale_general(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, size_t idx_norm, float power_t_norm);
float one_pf_quad_predict_trunc_rescale_general(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, float gravity, size_t idx_norm, float power_t_norm);

float offset_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, size_t offset);

void print_audit_quad(weight* weights, audit_data& page_feature, v_array<audit_data> &offer_features, size_t mask);
void print_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask);

float single_quad_weight(weight* weights, feature& page_feature, feature* offer_feature, size_t mask);

void cubic(v_array<feature> &f, const v_array<feature> &first_part, const v_array<feature> &second_part, const v_array<feature> &third_part, size_t mask);
float one_pf_cubic_predict(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask);
float one_pf_cubic_predict_trunc(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask, float gravity);
float one_pf_cubic_predict_rescale(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask, bool is_adaptive, size_t idx_norm);
float one_pf_cubic_predict_trunc_rescale(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask, float gravity, bool is_adaptive, size_t idx_norm);
float one_pf_cubic_predict_rescale_general(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask, size_t idx_norm, float power_t_norm);
float one_pf_cubic_predict_trunc_rescale_general(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask, float gravity, size_t idx_norm);
#endif
