/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
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



void cubic(v_array<feature> &f, const v_array<feature> &first_part, const v_array<feature> &second_part, const v_array<feature> &third_part, size_t mask)
{
  for (feature* i = first_part.begin; i != first_part.end; i++) {
    size_t firsthash = cubic_constant * cubic_constant2 * i->weight_index;
    float i_value = i->x;
    for (feature* ele = second_part.begin; ele != second_part.end; ele++) {
      size_t secondhash = firsthash + ele->weight_index * cubic_constant2;
      float j_value = i_value * ele->x;
      for (feature* ele2 = third_part.begin; ele2 != third_part.end; ele2++) {
        size_t thirdhash = (secondhash+ele2->weight_index) & mask;
        feature temp = {j_value * ele2->x, (uint32_t)thirdhash};
        push(f, temp);
      }
    }
  }
}

float one_pf_cubic_predict(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask)
{
  size_t halfhash = cubic_constant2 * (cubic_constant * f0.weight_index + f1.weight_index);
  return f0.x * f1.x * sd_offset_add(weights, mask, cross_features.begin, cross_features.end, halfhash);
}

float one_pf_cubic_predict_trunc(weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, size_t mask, float gravity)
{
  size_t halfhash = cubic_constant2 * (cubic_constant * f0.weight_index + f1.weight_index);  
  return f0.x * f1.x * sd_offset_add_trunc(weights, mask, cross_features.begin, cross_features.end, halfhash, gravity);
}
