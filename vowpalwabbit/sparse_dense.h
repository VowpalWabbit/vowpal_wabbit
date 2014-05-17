/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SPARSE_DENSE_H
#define SPARSE_DENSE_H

#include<math.h>

inline void vec_add(float& p, const float fx, float& fw) {
  p += fw * fx;
}

inline float sign(float w){ if (w < 0.) return -1.; else  return 1.;}

inline float trunc_weight(const float w, const float gravity){
  return (gravity < fabsf(w)) ? w - sign(w) * gravity : 0.f;
}

template<class R>
struct predict_data {
  float prediction;
  R extra;
};

inline void vec_add_trunc(predict_data<float>& p, const float fx, float& fw) {
  p.prediction += trunc_weight(fw, p.extra) * fx;
}

#endif
