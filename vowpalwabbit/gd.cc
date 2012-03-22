/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <fstream>
#include <sstream>
#include <float.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
#include <xmmintrin.h>
#endif

#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "cache.h"
#include "simple_label.h"
#include "allreduce.h"
#include "accumulate.h"

using namespace std;

void adaptive_inline_train(regressor &reg, example* &ec, float update);
void general_adaptive_train(regressor &reg, example* &ec, float update, float power_t);

size_t gd_current_pass = 0;

void learn_gd(example* ec)
{
  assert(ec->in_use);
  if (ec->pass != gd_current_pass)
    {
      if(global.span_server != "") {
	if(global.adaptive)
	  accumulate_weighted_avg(global.span_server, global.reg);
	else 
	  accumulate_avg(global.span_server, global.reg, 0);	      
      }
      
      if (global.save_per_pass)
	sync_weights(&global.reg);
      global.eta *= global.eta_decay_rate;
      save_predictor(global.final_regressor_name, gd_current_pass);
      gd_current_pass = ec->pass;
    }
  
  if (!command_example(ec))
    {
      predict(global.reg,ec);
      if (ec->eta_round != 0.)
	{
	  if (global.adaptive)
	    if (global.power_t == 0.5 || !global.exact_adaptive_norm)
	      adaptive_inline_train(global.reg,ec,ec->eta_round);
	    else
	      general_adaptive_train(global.reg,ec,ec->eta_round,global.power_t);
	  else
	    inline_train(global.reg, ec, ec->eta_round);
	  if (global.sd->contraction < 1e-10)  // updating weights now to avoid numerical instability
	    sync_weights(&global.reg);
	  
	}
    }
}

void finish_gd()
{
  sync_weights(&global.reg);
  if(global.span_server != "") {
    if(global.adaptive)
      accumulate_weighted_avg(global.span_server, global.reg);
    else 
      accumulate_avg(global.span_server, global.reg, 0);	      
  }
}

void sync_weights(regressor *reg) {
  if (global.sd->gravity == 0. && global.sd->contraction == 1.)  // to avoid unnecessary weight synchronization
    return;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length && global.reg_mode; i++)
    reg->weight_vectors[stride*i] = trunc_weight(reg->weight_vectors[stride*i], global.sd->gravity) * global.sd->contraction;
  global.sd->gravity = 0.;
  global.sd->contraction = 1.;
}

bool command_example(example* ec) {
  if (ec->indices.index() > 1)
    return false;

  if (ec->tag.index() >= 4 && !strncmp((const char*) ec->tag.begin, "save", 4))
    {//save state
      string final_regressor_name = global.final_regressor_name;

      if ((ec->tag).index() >= 6 && (ec->tag)[4] == '_')
	final_regressor_name = string(ec->tag.begin+5, (ec->tag).index()-5);

      if (!global.quiet)
	cerr << "saving regressor to " << final_regressor_name << endl;
      dump_regressor(final_regressor_name, global.reg);

      return true;
    }
  return false;
}

float finalize_prediction(float ret) 
{
  if ( isnan(ret))
    {
      cout << "you have a NAN!!!!!" << endl;
      return 0.;
    }
  if ( ret > global.sd->max_label )
    return global.sd->max_label;
  if (ret < global.sd->min_label)
    return global.sd->min_label;

  return ret;
}

void finish_example(example* ec)
{
  return_simple_example(ec);
}

float query_decision(example*, float k);

float inline_predict_trunc(regressor &reg, example* &ec)
{
  float prediction = global.lp->get_initial(ec->ld);
  
  weight* weights = reg.weight_vectors;
  size_t mask = global.weight_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    prediction += sd_add_trunc(weights,mask,ec->atomics[*i].begin, ec->atomics[*i].end, global.sd->gravity);
  
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    prediction += one_pf_quad_predict_trunc(weights, *temp.begin,
						    ec->atomics[(int)(*i)[1]], mask, global.sd->gravity);
	}
    }
  
  return prediction;
}

float inline_predict(regressor &reg, example* &ec)
{
  float prediction = global.lp->get_initial(ec->ld);

  weight* weights = reg.weight_vectors;
  size_t mask = global.weight_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    prediction += sd_add(weights,mask,ec->atomics[*i].begin, ec->atomics[*i].end);
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    prediction += one_pf_quad_predict(weights,*temp.begin,
					      ec->atomics[(int)(*i)[1]],mask);
	}
    }
  
  return prediction;
}

struct string_value {
  float v;
  string s;
  friend bool operator<(const string_value& first, const string_value& second);
};

bool operator<(const string_value& first, const string_value& second)
{
  return fabs(first.v) > fabs(second.v);
}

#include <algorithm>

void print_audit_quad(weight* weights, audit_data& page_feature, v_array<audit_data> &offer_features, vector<string_value>& features)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;

  for (audit_data* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      ostringstream tempstream;
      tempstream << '\t' << page_feature.space << '^' << page_feature.feature << '^' 
		 << ele->space << '^' << ele->feature << ':' << (((halfhash + ele->weight_index)/global.stride) & global.parse_mask)
		 << ':' << ele->x*page_feature.x
		 << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & global.weight_mask], global.sd->gravity) * global.sd->contraction;
      string_value sv = {weights[ele->weight_index & global.weight_mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

void print_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, vector<string_value>& features)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      ostringstream tempstream;
      cout << '\t' << (((halfhash + ele->weight_index)/global.stride) & global.parse_mask) 
	   << ':' << (ele->x*page_feature.x)
	   << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & global.weight_mask], global.sd->gravity) * global.sd->contraction;
      string_value sv = {weights[ele->weight_index & global.weight_mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

void print_features(regressor &reg, example* &ec)
{
  weight* weights = reg.weight_vectors;
  size_t stride = global.stride;

  if (global.lda > 0)
    {
      size_t count = 0;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	count += ec->audit_features[*i].index() + ec->atomics[*i].index();
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	  {
	    cout << '\t' << f->space << '^' << f->feature << ':' << (f->weight_index/global.stride & global.parse_mask) << ':' << f->x;
	    for (size_t k = 0; k < global.lda; k++)
	      cout << ':' << weights[(f->weight_index+k) & global.weight_mask];
	  }
      cout << " total of " << count << " features." << endl;
    }
  else
    {
      vector<string_value> features;

      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
	  for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	    {
	      ostringstream tempstream;
	      tempstream << f->space << '^' << f->feature << ':' << (f->weight_index/stride & global.parse_mask) << ':' << f->x;
	      tempstream  << ':' << trunc_weight(weights[f->weight_index & global.weight_mask], global.sd->gravity) * global.sd->contraction;
	      if(global.adaptive)
		tempstream << '@' << weights[(f->weight_index+1) & global.weight_mask];
	      string_value sv = {weights[f->weight_index & global.weight_mask]*f->x, tempstream.str()};
	      features.push_back(sv);
	    }
	else
	  for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	    {
	      ostringstream tempstream;
	      if ( f->weight_index == ((constant*stride)&global.weight_mask))
		tempstream << "Constant:";
	      tempstream << (f->weight_index/stride & global.parse_mask) << ':' << f->x;
	      tempstream << ':' << trunc_weight(weights[f->weight_index & global.weight_mask], global.sd->gravity) * global.sd->contraction;
	      if(global.adaptive)
		tempstream << '@' << weights[(f->weight_index+1) & global.weight_mask];
	      string_value sv = {weights[f->weight_index & global.weight_mask]*f->x, tempstream.str()};
	      features.push_back(sv);
	    }
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
	if (ec->audit_features[(int)(*i)[0]].begin != ec->audit_features[(int)(*i)[0]].end)
	  for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f != ec->audit_features[(int)(*i)[0]].end; f++)
	    print_audit_quad(weights, *f, ec->audit_features[(int)(*i)[1]], features);
	else
	  for (feature* f = ec->atomics[(int)(*i)[0]].begin; f != ec->atomics[(int)(*i)[0]].end; f++)
	    print_quad(weights, *f, ec->atomics[(int)(*i)[1]], features);      

      sort(features.begin(),features.end());

      for (vector<string_value>::iterator sv = features.begin(); sv!= features.end(); sv++)
	cout << '\t' << (*sv).s;
      cout << endl;
    }
}

void print_audit_features(regressor &reg, example* ec)
{
  fflush(stdout);
  print_result(fileno(stdout),ec->final_prediction,-1,ec->tag);
  fflush(stdout);
  print_features(reg, ec);
}

void one_pf_quad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  update *= page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    weights[(halfhash + ele->weight_index) & mask] += update * ele->x;
}

float InvSqrt(float x){
  float xhalf = 0.5f * x;
  int i = *(int*)&x; // store floating-point bits in integer
  i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
  x = *(float*)&i; // convert new bits into float
  x = x*(1.5f - xhalf*x*x); // One round of Newton's method
  return x;
}

void one_pf_quad_adaptive_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, float g, example* ec)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  update *= page_feature.x;
  float update2 = g * page_feature.x * page_feature.x;

  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[1] += update2 * ele->x * ele->x;
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
      float t;
      __m128 eta = _mm_load_ss(&w[1]);
      eta = _mm_rsqrt_ss(eta);
      _mm_store_ss(&t, eta);
      t *= ele->x;
#else
      float t = ele->x*InvSqrt(w[1]);
#endif
      w[0] += update * t;
    }
}

void offset_quad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, size_t offset)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index + offset;
  update *= page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    weights[(halfhash + ele->weight_index) & mask] += update * ele->x;
}

void adaptive_inline_train(regressor &reg, example* &ec, float update)
{
  if (fabs(update) == 0.)
    return;

  size_t mask = global.weight_mask;
  label_data* ld = (label_data*)ec->ld;
  weight* weights = reg.weight_vectors;
  
  float g = global.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++)
	{
	  weight* w = &weights[f->weight_index & mask];
	  w[1] += g * f->x * f->x;
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
      float t;
      __m128 eta = _mm_load_ss(&w[1]);
      eta = _mm_rsqrt_ss(eta);
      _mm_store_ss(&t, eta);
      t *= f->x;
#else
	  float t = f->x*InvSqrt(w[1]);
#endif
	  w[0] += update * t;
	}
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    one_pf_quad_adaptive_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, update, g, ec);
	} 
    }
}

void quad_general_adaptive_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, float g, example* ec, float power_t)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  update *= page_feature.x;
  float update2 = g * page_feature.x * page_feature.x;
  
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[1] += update2 * ele->x * ele->x;
      float t = ele->x*powf(w[1],-power_t);
      w[0] += update * t;
    }
}

void general_adaptive_train(regressor &reg, example* &ec, float update, float power_t)
{
  if (fabs(update) == 0.)
    return;
  
  size_t mask = global.weight_mask;
  label_data* ld = (label_data*)ec->ld;
  weight* weights = reg.weight_vectors;
  
  float g = global.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++)
	{
	  weight* w = &weights[f->weight_index & mask];
	  w[1] += g * f->x * f->x;
	  float t = f->x*powf(w[1],-power_t);
	  w[0] += update * t;
	}
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    quad_general_adaptive_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, update, g, ec, power_t);
	} 
    }
}


float xGx_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float g)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float xGx = 0.;
  float update2 = g * page_feature.x * page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
      float m = w[1] + update2 * ele->x * ele->x;
      __m128 eta = _mm_load_ss(&m);
      eta = _mm_rsqrt_ss(eta);
      _mm_store_ss(&m, eta);
      float t = ele->x * m;
#else
      float t = ele->x*InvSqrt(w[1] + update2 * ele->x * ele->x);
#endif
      xGx += t * ele->x;
    }
  return xGx;
}

float xGx_general_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float g, float power_t)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float xGx = 0.;
  float update2 = g * page_feature.x * page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      float t = ele->x*powf(w[1] + update2 * ele->x * ele->x,- power_t);
      xGx += t * ele->x;
    }
  return xGx;
}

float compute_general_xGx(regressor &reg, example* &ec, float power_t)
{//We must traverse the features in _precisely_ the same order as during training.
  size_t mask = global.weight_mask;
  label_data* ld = (label_data*)ec->ld;
  float g = global.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  if (g==0) return 1.;

  float xGx = 0.;
  
  weight* weights = reg.weight_vectors;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++)
	{
	  weight* w = &weights[f->weight_index & mask];
	  float t = f->x*powf(w[1] + g * f->x * f->x,- power_t);
	  xGx += t * f->x;
	}
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    xGx += xGx_general_quad(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, g, power_t);
	} 
    }
  
  return xGx;
}

float compute_xGx(regressor &reg, example* &ec)
{//We must traverse the features in _precisely_ the same order as during training.
  size_t mask = global.weight_mask;
  label_data* ld = (label_data*)ec->ld;
  float g = global.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  if (g==0) return 1.;

  float xGx = 0.;
  
  weight* weights = reg.weight_vectors;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++)
	{
	  weight* w = &weights[f->weight_index & mask];
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
      float m = w[1] + g * f->x * f->x;
      __m128 eta = _mm_load_ss(&m);
      eta = _mm_rsqrt_ss(eta);
      _mm_store_ss(&m, eta);
      float t = f->x * m;
#else
	  float t = f->x*InvSqrt(w[1] + g * f->x * f->x);
#endif
	  xGx += t * f->x;
	}
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    xGx += xGx_quad(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, g);
	} 
    }
  
  return xGx;
}

void inline_train(regressor &reg, example* &ec, float update)
{
  if (fabs(update) == 0.)
    return;
  size_t mask = global.weight_mask;
  weight* weights = reg.weight_vectors;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++){
	weights[f->weight_index & mask] += update * f->x;
      }
    }
  
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    one_pf_quad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, update);
	} 
    }
}

void train(weight* weights, const v_array<feature> &features, float update)
{
  if (fabs(update) > 0.)
    for (feature* j = features.begin; j != features.end; j++)
      weights[j->weight_index] += update * j->x;
}

void local_predict(example* ec, regressor& reg)
{
  label_data* ld = (label_data*)ec->ld;

  set_minmax(ld->label);

  ec->final_prediction = finalize_prediction(ec->partial_prediction * global.sd->contraction);

  if(global.active_simulation){
    float k = ec->example_t - ld->weight;
    ec->revert_weight = global.loss->getRevertingWeight(ec->final_prediction, global.eta/powf(k,global.power_t));
    float importance = query_decision(ec, k);
    if(importance > 0){
      global.sd->queries += 1;
      ld->weight *= importance;
    }
    else //do not query => do not train
      ld->label = FLT_MAX;
  }

  float t;
  if(global.active)
    t = global.sd->weighted_unlabeled_examples;
  else
    t = ec->example_t;

  ec->eta_round = 0;
  if (ld->label != FLT_MAX)
    {
      ec->loss = global.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;

      if (global.training && ec->loss > 0.)
	{
	  double eta_t;
	  float norm;
	  if (global.adaptive && global.exact_adaptive_norm) {
	    float magx = 0.;
	    if (global.power_t == 0.5)
	      norm = compute_xGx(reg, ec);
	    else 
	      norm = compute_general_xGx(reg,ec, global.power_t);
	    magx = powf(ec->total_sum_feat_sq, 1. - global.power_t);
	    eta_t = global.eta * norm / magx * ld->weight;
	  } else {
	    eta_t = global.eta / powf(t,global.power_t) * ld->weight;
	    if (global.nonormalize) 
	      {
		norm = 1.;
		eta_t *= ec->total_sum_feat_sq;
	      }
	    else
	      norm = ec->total_sum_feat_sq;
	  }
	  ec->eta_round = global.loss->getUpdate(ec->final_prediction, ld->label, eta_t, norm) / global.sd->contraction;

	  if (global.reg_mode && fabs(ec->eta_round) > 1e-8) {
	    double dev1 = global.loss->first_derivative(ec->final_prediction, ld->label);
	    double eta_bar = (fabs(dev1) > 1e-8) ? (-ec->eta_round / dev1) : 0.0;
	    if (fabs(dev1) > 1e-8)
	      global.sd->contraction /= (1. + global.l2_lambda * eta_bar * norm);
	    global.sd->gravity += eta_bar * sqrt(norm) * global.l1_lambda;
	  }
	}
    }
  else if(global.active)
    ec->revert_weight = global.loss->getRevertingWeight(ec->final_prediction, global.eta/powf(t,global.power_t));

  if (global.audit)
    print_audit_features(reg, ec);
}

void predict(regressor& r, example* ex)
{
  float prediction;
  if (global.reg_mode % 2)
    prediction = inline_predict_trunc(r, ex);
  else
    prediction = inline_predict(r, ex);

  ex->partial_prediction += prediction;

  local_predict(ex, r);
  ex->done = true;
}

// trains regressor r on one example ex.
void train_one_example(regressor& r, example* ex)
{
  predict(r,ex);
  label_data* ld = (label_data*) ex->ld;
  if (ld->label != FLT_MAX && global.training) 
    inline_train(r, ex, ex->eta_round);
}

void drive_gd()
{
  example* ec = NULL;
  
  while ( true )
    {
      if ((ec = get_example()) != NULL)//semiblocking operation.
	{
	  learn_gd(ec);
	  finish_example(ec);
	}
      else if (parser_done())
	{
	  finish_gd();
	  return;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
}
