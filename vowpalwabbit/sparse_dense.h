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

template<class R> 
struct predict_data {
  float prediction;
  R extra;
};
  
inline float sign(float w){ if (w < 0.) return -1.; else  return 1.;}

inline float trunc_weight(float w, float gravity){
  return (gravity < fabsf(w)) ? w - sign(w) * gravity : 0.f;
}

inline void vec_add(vw& all, predict_data<float>& p, float fx, uint32_t fi) {
  p.prediction += all.reg.weight_vector[fi & all.reg.weight_mask] * fx;
}

inline void vec_add_trunc(vw& all, predict_data<float>& p, float fx, uint32_t fi) {
  p.prediction += trunc_weight(all.reg.weight_vector[fi & all.reg.weight_mask], p.extra) * fx;
}

template <bool adaptive, size_t normalized_idx>
inline void vec_add_rescale(vw& all, predict_data<float>& p, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
  float x_abs = fabs(fx);
  if( x_abs > w[normalized_idx] ) {// new scale discovered
    if( w[normalized_idx] > 0. ) {//If the normalizer is > 0 then rescale the weight so it's as if the new scale was the old scale.
      float rescale = (w[normalized_idx]/x_abs);
      w[0] *= (adaptive ? rescale : rescale*rescale);
    }
    w[normalized_idx] = x_abs;
  }
  p.prediction += w[0] * fx;
}

template <bool adaptive, size_t normalized_idx>
inline void vec_add_trunc_rescale(vw& all, predict_data<float>& p, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
  float x_abs = fabs(fx);
  if( x_abs > w[normalized_idx] ) {
    if( w[normalized_idx] > 0. ) {
      float rescale = (w[normalized_idx]/x_abs);
      w[0] *= (adaptive ? rescale : rescale*rescale);
    }
    w[normalized_idx] = x_abs;
  }
  p.prediction += trunc_weight(w[0], p.extra) * fx;
}

template <bool adaptive, size_t normalized_idx>
inline void vec_add_rescale_general(vw& all, predict_data<float>& p, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
  float x_abs = fabs(fx);
  float power_t_norm = 1.f - (adaptive ? p.extra : 0.f);
  if( x_abs > w[normalized_idx] ) {
    if( w[normalized_idx] > 0. ) {
      float rescale = (w[normalized_idx]/x_abs);
      w[0] *= powf(rescale*rescale,power_t_norm);
    }
    w[normalized_idx] = x_abs;
  }
  p.prediction += w[0] * fx;
}

struct gnp {
  float gravity;
  float power_t;
};

template <bool adaptive, size_t normalized_idx>
inline void vec_add_trunc_rescale_general(vw& all, predict_data<gnp>& p, float fx, uint32_t fi) {
  weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
  float x_abs = fabs(fx);
  float power_t_norm = 1.f - (adaptive ? p.extra.power_t : 0.f);
  if( x_abs > w[normalized_idx] ) {
    if( w[normalized_idx] > 0. ) {
      float rescale = (w[normalized_idx]/x_abs);
      w[0] *= powf(rescale*rescale,power_t_norm);
    }
    w[normalized_idx] = x_abs;
  }
  p.prediction += trunc_weight(w[0], p.extra.gravity) * fx;
}

void sd_offset_update(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float update, float regularization);

#endif
