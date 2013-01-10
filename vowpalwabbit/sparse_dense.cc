/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */

#include "sparse_dense.h"
#include "constant.h"
#include <math.h>

float sd_offset_add_trunc(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float gravity)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++)
    ret += trunc_weight(weights[(f->weight_index + offset) & mask], gravity) * f->x;
  return ret;
}

float sd_offset_add_rescale(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float x, bool is_adaptive, size_t idx_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[(f->weight_index + offset) & mask];
    float xtmp = f->x;
    float xquad_abs = fabs(x*xtmp);
    if( xquad_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0. ) {
        float rescale = w[idx_norm]/xquad_abs;
        w[0] *= (is_adaptive ? rescale : rescale*rescale);
      }
      w[idx_norm] = xquad_abs;
    }
    ret += w[0] * xtmp;
  }
  return ret;
}

float sd_offset_add_trunc_rescale(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float gravity, float x, bool is_adaptive, size_t idx_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[(f->weight_index + offset) & mask];
    float xtmp = f->x;
    float xquad_abs = fabs(x*xtmp);
    if( xquad_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0. ) {
        float rescale = w[idx_norm]/xquad_abs;
        w[0] *= (is_adaptive ? rescale : rescale*rescale);
      }
      w[idx_norm] = xquad_abs;
    }
    ret += trunc_weight(w[0],gravity) * xtmp;
  }
  return ret;
}

float sd_offset_add_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float x, size_t idx_norm, float power_t_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[(f->weight_index + offset) & mask];
    float xtmp = f->x;
    float xquad_abs = fabs(x*xtmp);
    if( xquad_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0 ) {
        float rescale = w[idx_norm]/xquad_abs;
        w[0] *= powf(rescale*rescale,power_t_norm);
      }
      w[idx_norm] = xquad_abs;
    }
    ret += w[0] * xtmp;
  }
  return ret;
}

float sd_offset_add_trunc_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float gravity, float x, size_t idx_norm, float power_t_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[(f->weight_index + offset) & mask];
    float xtmp = f->x;
    float xquad_abs = fabs(x*xtmp);
    if( xquad_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0 ) {
        float rescale = w[idx_norm]/xquad_abs;
        w[0] *= powf(rescale*rescale,power_t_norm);
      }
      w[idx_norm] = xquad_abs;
    }
    ret += trunc_weight(w[0],gravity) * xtmp;
  }
  return ret;
}

void sd_offset_update(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float update, float regularization)
{
  for (feature* f = begin; f!= end; f++) 
    weights[(f->weight_index + offset) & mask] += update * f->x - regularization * weights[(f->weight_index + offset) & mask];
} 

void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t mask)
{
  for (feature* i = first_part.begin; i != first_part.end; i++)
    {
      size_t halfhash = quadratic_constant * i->weight_index;
      float i_value = i->x;
      for (feature* ele = second_part.begin; ele != second_part.end; ele++) {
        size_t quad_index = (halfhash+ele->weight_index) & mask;
        feature temp = {i_value * ele->x, (uint32_t)quad_index};
        f.push_back(temp);
      }
    }
}

float one_of_quad_predict(v_array<feature> &page_features, feature& offer_feature, weight* weights, size_t mask)
{
  float prediction = 0.0;
  for (feature* i = page_features.begin; i != page_features.end; i++)
    {
      size_t halfhash = quadratic_constant * i->weight_index;
      prediction += weights[(halfhash + offer_feature.weight_index) & mask] * i->x;
    }
  return prediction * offer_feature.x;
}
