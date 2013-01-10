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

template <void (*T)(vw&,float,uint32_t,float&)>
void sd_add(vw& all, feature* begin, feature* end, float& ret)
{
  for (feature* f = begin; f!= end; f++)
    T(all, f->x, f->weight_index, ret);
}

inline void vec_add(vw& all, float fx, uint32_t fi, float& ret) {
  ret += all.reg.weight_vectors[fi & all.weight_mask] * fx;
}

inline void vec_add_trunc(vw& all, float fx, uint32_t fi, float& ret) {
  ret += trunc_weight(all.reg.weight_vectors[fi & all.weight_mask], all.sd->gravity) * fx;
}

inline void vec_add_rescale(vw& all, float fx, uint32_t fi, float& ret) {
  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float x_abs = fabs(fx);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= (all.adaptive ? rescale : rescale*rescale);
    }
    w[all.normalized_idx] = x_abs;
  }
  ret += w[0] * fx;
}

inline void vec_add_trunc_rescale(vw& all, float fx, uint32_t fi, float& ret) {
  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float x_abs = fabs(fx);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= (all.adaptive ? rescale : rescale*rescale);
    }
    w[all.normalized_idx] = x_abs;
  }
  ret += trunc_weight(w[0], all.sd->gravity) * fx;
}

inline void vec_add_rescale_general(vw& all, float fx, uint32_t fi, float& ret) {
  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float x_abs = fabs(fx);
  float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= powf(rescale*rescale,power_t_norm);
    }
    w[all.normalized_idx] = x_abs;
  }
  ret += w[0] * fx;
}

inline void vec_add_trunc_rescale_general(vw& all, float fx, uint32_t fi, float& ret) {
  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float x_abs = fabs(fx);
  float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= powf(rescale*rescale,power_t_norm);
    }
    w[all.normalized_idx] = x_abs;
  }
  ret += trunc_weight(w[0], all.sd->gravity) * fx;
}



/*
float sd_add(weight* weights, size_t mask, feature* begin, feature* end);
float sd_add_trunc(weight* weights, size_t mask, feature* begin, feature* end, float gravity);
float sd_add_rescale(weight* weights, size_t mask, feature* begin, feature* end, bool is_adaptive, size_t idx_norm);
float sd_add_trunc_rescale(weight* weights, size_t mask, feature* begin, feature* end, float gravity, bool is_adaptive, size_t idx_norm);
float sd_add_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, size_t idx_norm, float power_t_norm);
float sd_add_trunc_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, float gravity, size_t idx_norm, float power_t_norm);
*/

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
