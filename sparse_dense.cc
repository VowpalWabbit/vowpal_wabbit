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
  for (feature* f = begin; f!= end; f++)
    ret += weights[f->weight_index & mask] * f->x;
  return ret;
}

float sd_offset_add(weight* weights, size_t mask, feature* begin, feature* end, size_t offset)
{
  float ret = 0.;
  for (feature* f = begin; f!= end; f++)
    ret += weights[(f->weight_index + offset) & mask] * f->x;
  return ret;
}

void sd_offset_update(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float update)
{
  float mult = 1 - global.weight_decay_sparse;
  float new_weight;
  for (feature* f = begin; f!= end; f++) {
    // w <- eta*(y-yhat) + (1-lambda)w
    new_weight = f->x * update + mult * weights[(f->weight_index + offset) & mask];
    weights[(f->weight_index + offset) & mask] = new_weight;
  }
} 

void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t thread_mask)
{
  for (feature* i = first_part.begin; i != first_part.end; i++)
    {
      size_t halfhash = quadratic_constant * i->weight_index;
      float i_value = i->x;
      for (feature* ele = second_part.begin; ele != second_part.end; ele++) {
        size_t quad_index = (halfhash+ele->weight_index) & thread_mask;
        feature temp = {i_value * ele->x, quad_index};
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
  
  return f.x * 
    sd_offset_add(weights, mask, cross_features.begin, cross_features.end, halfhash);
}

float offset_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, size_t offset)
{
  float prediction = 0.0;
  size_t halfhash = quadratic_constant * page_feature.weight_index + offset;

  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    prediction += weights[(halfhash + ele->weight_index) & mask] * ele->x;

  return (prediction*page_feature.x);
}

void print_audit_quad(weight* weights, audit_data& page_feature, v_array<audit_data> &offer_features, size_t mask)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;

  for (audit_data* ele = offer_features.begin; ele != offer_features.end; ele++)
    cout << '\t' << page_feature.space << '^' << page_feature.feature << '^' 
	 << ele->space << '^' << ele->feature << ':' << (((halfhash + ele->weight_index)/global.stride) & mask)
	 << ':' << ele->x*page_feature.x
	 << ':' << weights[(halfhash + ele->weight_index) & mask];
}

void print_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    cout << '\t' << (((halfhash + ele->weight_index)/global.stride) & mask) 
	 << ':' << (ele->x*page_feature.x)
	 << ':' << weights[(halfhash + ele->weight_index) & mask];
}

float single_quad_weight(weight* weights, feature& page_feature, feature* offer_feature, size_t mask)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float quad_weight = weights[(halfhash + offer_feature->weight_index) & mask] * offer_feature->x;
  return (quad_weight*page_feature.x);
}
