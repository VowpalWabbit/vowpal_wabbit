/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */

#include "sparse_dense.h"
#include "constant.h"
#include <math.h>

float sd_add(weight* weights, size_t mask, feature* begin, feature* end)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    ret += weights[f->weight_index & mask] * f->x;
  }
  return ret;
}

float sd_add_trunc(weight* weights, size_t mask, feature* begin, feature* end, float gravity)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++)
      ret += trunc_weight(weights[f->weight_index & mask], gravity) * f->x;
  return ret;
}

float sd_add_rescale(weight* weights, size_t mask, feature* begin, feature* end, bool is_adaptive, size_t idx_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[f->weight_index & mask];
    float x = f->x;
    float x_abs = fabs(x);
    if( x_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0. ) {
        float rescale = (w[idx_norm]/x_abs);
        w[0] *= (is_adaptive ? rescale : rescale*rescale);
      }
      w[idx_norm] = x_abs;
    }
    ret += w[0] * x;
  }
  return ret;
}

float sd_add_trunc_rescale(weight* weights, size_t mask, feature* begin, feature* end, float gravity, bool is_adaptive, size_t idx_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[f->weight_index & mask];
    float x = f->x;
    float x_abs = fabs(x);
    if( x_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0. ) {
        float rescale = (w[idx_norm]/x_abs);
        w[0] *= (is_adaptive ? rescale : rescale*rescale);
      }
      w[idx_norm] = x_abs;
    }
    ret += trunc_weight(w[0], gravity) * x;
  }
  return ret;
}

float sd_add_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, size_t idx_norm, float power_t_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[f->weight_index & mask];
    float x = f->x;
    float x_abs = fabs(x);
    if( x_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0. ) {
        float rescale = (w[idx_norm]/x_abs);
        w[0] *= powf(rescale*rescale,power_t_norm);
      }
      w[idx_norm] = x_abs;
    }
    ret += w[0] * x;
  }
  return ret;
}

float sd_add_trunc_rescale_general(weight* weights, size_t mask, feature* begin, feature* end, float gravity, size_t idx_norm, float power_t_norm)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++) {
    weight* w = &weights[f->weight_index & mask];
    float x = f->x;
    float x_abs = fabs(x);
    if( x_abs > w[idx_norm] ) {
      if( w[idx_norm] > 0. ) {
        float rescale = (w[idx_norm]/x_abs);
        w[0] *= powf(rescale*rescale,power_t_norm);
      }
      w[idx_norm] = x_abs;
    }
    ret += trunc_weight(w[0], gravity) * x;
  }
  return ret;
}

float sd_offset_add(weight* weights, size_t mask, feature* begin, feature* end, size_t offset)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++)
    ret += weights[(f->weight_index + offset) & mask] * f->x;
  return ret;
}

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
        push(f, temp);
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

float one_pf_quad_predict(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask)
{
  size_t halfhash = quadratic_constant * f.weight_index;
  
  return f.x * sd_offset_add(weights, mask, cross_features.begin, cross_features.end, halfhash);
}

float one_pf_quad_predict_trunc(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, float gravity)
{
  size_t halfhash = quadratic_constant * f.weight_index;
  
  return f.x * sd_offset_add_trunc(weights, mask, cross_features.begin, cross_features.end, halfhash, gravity);
}

float one_pf_quad_predict_rescale(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, bool is_adaptive, size_t idx_norm)
{
  size_t halfhash = quadratic_constant * f.weight_index;
  
  return f.x * sd_offset_add_rescale(weights, mask, cross_features.begin, cross_features.end, halfhash, f.x, is_adaptive, idx_norm);
}

float one_pf_quad_predict_trunc_rescale(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, float gravity, bool is_adaptive, size_t idx_norm)
{
  size_t halfhash = quadratic_constant * f.weight_index;
  
  return f.x * sd_offset_add_trunc_rescale(weights, mask, cross_features.begin, cross_features.end, halfhash, gravity, f.x, is_adaptive, idx_norm);
}

float one_pf_quad_predict_rescale_general(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, size_t idx_norm, float power_t_norm)
{
  size_t halfhash = quadratic_constant * f.weight_index;
  
  return f.x * sd_offset_add_rescale_general(weights, mask, cross_features.begin, cross_features.end, halfhash, f.x, idx_norm, power_t_norm);
}

float one_pf_quad_predict_trunc_rescale_general(weight* weights, feature& f, v_array<feature> &cross_features, size_t mask, float gravity, size_t idx_norm, float power_t_norm)
{
  size_t halfhash = quadratic_constant * f.weight_index;
  
  return f.x * sd_offset_add_trunc_rescale_general(weights, mask, cross_features.begin, cross_features.end, halfhash, gravity, f.x, idx_norm, power_t_norm);
}

float offset_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, size_t offset)
{
  float prediction = 0.0;
  size_t halfhash = quadratic_constant * page_feature.weight_index + offset;

  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    prediction += weights[(halfhash + ele->weight_index) & mask] * ele->x;

  return (prediction*page_feature.x);
}

float single_quad_weight(weight* weights, feature& page_feature, feature* offer_feature, size_t mask)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float quad_weight = weights[(halfhash + offer_feature->weight_index) & mask] * offer_feature->x;
  return (quad_weight*page_feature.x);
}
