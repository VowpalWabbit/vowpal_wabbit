/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SPARSE_DENSE_VECTOR_H
#define SPARSE_DENSE_VECTOR_H

#include <math.h>
#include "parse_example.h"
#include "constant.h"

inline float sign(float w){ if (w < 0.) return -1.; else  return 1.;}

inline float trunc_weight(float w, float gravity){
  return (gravity < fabsf(w)) ? w - sign(w) * gravity : 0.f;
}

template <float (*T)(vw&,float,uint32_t)>
float sd_add(vw& all, feature* begin, feature* end, uint32_t offset=0)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++)
    ret += T(all, f->x, f->weight_index + offset);
  return ret;
}

template <float (*T)(vw&,float,uint32_t)>
float one_pf_quad_predict(vw& all, feature& f, v_array<feature> cross_features, uint32_t offset=0)
{
  uint32_t halfhash = quadratic_constant * (f.weight_index + offset);
  return f.x * sd_add<T>(all, cross_features.begin, cross_features.end, halfhash + offset);
}

template <float (*T)(vw&,float,uint32_t)>
float one_pf_cubic_predict(vw& all, feature& f0, feature& f1, v_array<feature> cross_features, uint32_t offset=0)
{
  uint32_t halfhash = cubic_constant2 * (cubic_constant * (f0.weight_index + offset) + f1.weight_index + offset);
  return f0.x * f1.x * sd_add<T>(all, cross_features.begin, cross_features.end, halfhash + offset);
}

inline float vec_add(vw& all, float fx, uint32_t fi) {
  return all.reg.weight_vector[fi & all.weight_mask] * fx;
}

inline float vec_add_trunc(vw& all, float fx, uint32_t fi) {
  return trunc_weight(all.reg.weight_vector[fi & all.weight_mask], (float)all.sd->gravity) * fx;
}

inline float vec_add_rescale(vw& all, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.weight_mask];
  float x_abs = fabs(fx);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= (all.adaptive ? rescale : rescale*rescale);
    }
    w[all.normalized_idx] = x_abs;
  }
  return w[0] * fx;
}

inline float vec_add_trunc_rescale(vw& all, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.weight_mask];
  float x_abs = fabs(fx);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= (all.adaptive ? rescale : rescale*rescale);
    }
    w[all.normalized_idx] = x_abs;
  }
  return trunc_weight(w[0], (float)all.sd->gravity) * fx;
}

inline float vec_add_rescale_general(vw& all, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.weight_mask];
  float x_abs = fabs(fx);
  float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= powf(rescale*rescale,power_t_norm);
    }
    w[all.normalized_idx] = x_abs;
  }
  return w[0] * fx;
}

inline float vec_add_trunc_rescale_general(vw& all, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.weight_mask];
  float x_abs = fabs(fx);
  float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
  if( x_abs > w[all.normalized_idx] ) {
    if( w[all.normalized_idx] > 0. ) {
      float rescale = (w[all.normalized_idx]/x_abs);
      w[0] *= powf(rescale*rescale,power_t_norm);
    }
    w[all.normalized_idx] = x_abs;
  }
  return trunc_weight(w[0], (float)all.sd->gravity) * fx;
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <void (*T)(vw&,float,uint32_t,float)>
void sd_update(vw& all, feature* begin, feature* end, float update, uint32_t offset=0)
{
  for (feature* f = begin; f!= end; f++)
    T(all, f->x, f->weight_index + offset, update);
}

template <void (*T)(vw&,float,uint32_t,float)>
void sd_quad_update(vw& all, feature& f, v_array<feature> cross_features, float update, uint32_t offset=0)
{
  size_t halfhash = quadratic_constant * (f.weight_index + offset);
  sd_update<T>(all, cross_features.begin, cross_features.end, halfhash + offset, update * f.x);
}

template <void (*T)(vw&,float,uint32_t,float)>
void sd_cubic_update(vw& all, feature& f0, feature& f1, v_array<feature> cross_features, float update, uint32_t offset=0)
{
  size_t halfhash = cubic_constant2 * (cubic_constant * (f0.weight_index + offset) + f1.weight_index + offset);
  sd_update<T>(all, cross_features.begin, cross_features.end, update * f0.x * f1.x, halfhash + offset);
}

inline void upd_add(vw& all, float fx, uint32_t fi, float update) {
  all.reg.weight_vector[fi] += update * fx;
}


void sd_offset_update(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float update, float regularization);

void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t thread_mask);

#endif
