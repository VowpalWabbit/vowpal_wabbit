/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <fstream>
#include <sstream>
#include <float.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netdb.h>
#endif
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

//nonreentrant
size_t gd_current_pass = 0;

void predict(vw& all, example* ex);
void sync_weights(vw& all);
void general_train(vw& all, example* &ec, float update, float power_t);
void inline_train(vw& all, example* &ec, float update);


template <void (*T)(vw&, float, uint32_t, float, float)>
void generic_train(vw& all, example* &ec, float update, uint32_t offset=0)
{
  if (fabs(update) == 0.)
    return;

  float total_weight = 0.f;
  if(all.active)
    total_weight = (float)all.sd->weighted_unlabeled_examples;
  else
    total_weight = ec->example_t;

  float avg_norm = sqrt(all.normalized_sum_norm_x / total_weight);
  label_data* ld = (label_data*)ec->ld;
  float g = all.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;

  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
    for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
      T(all, f->x, f->weight_index, avg_norm, update);

  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
    if ((ec->atomics[(int)(*i)[0]].size() > 0) && (ec->atomics[(int)(*i)[1]].size() > 0))
      for (feature* f0 = ec->atomics[(int)(*i)[0]].begin; f0 != ec->atomics[(int)(*i)[0]].end; f0++) {
        size_t halfhash = quadratic_constant * (f0->weight_index);
        float update2 = update * f0->x;
        for (feature* ele = ec->atomics[(int)(*i)[1]].begin; ele != ec->atomics[(int)(*i)[1]].end; ele++)
          T(all, ele->x, ele->weight_index + halfhash, avg_norm, update2);
      }

  for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) 
    if ((ec->atomics[(int)(*i)[0]].size() > 0) && (ec->atomics[(int)(*i)[1]].size() > 0) && (ec->atomics[(int)(*i)[2]].size() > 0))
      for (feature* f0 = ec->atomics[(int)(*i)[0]].begin; f0 != ec->atomics[(int)(*i)[0]].end; f0++)
        for (feature* f1 = ec->atomics[(int)(*i)[1]].begin; f1 != ec->atomics[(int)(*i)[1]].end; f1++) {
          size_t halfhash = cubic_constant2 * (cubic_constant * (f0->weight_index + offset) + f1->weight_index + offset);
          float update2 = update * f0->x * f1->x;
          for (feature* ele = ec->atomics[(int)(*i)[2]].begin; ele != ec->atomics[(int)(*i)[2]].end; ele++)
            T(all, ele->x, ele->weight_index + halfhash, avg_norm, update2);
        }
}

float InvSqrt(float x){
  float xhalf = 0.5f * x;
  int i = *(int*)&x; // store floating-point bits in integer
  i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
  x = *(float*)&i; // convert new bits into float
  x = x*(1.5f - xhalf*x*x); // One round of Newton's method
  return x;
}

inline void general_update(vw& all, float x, uint32_t fi, float avg_norm, float update)
{
  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
  float t = 1.f;
  if(all.adaptive) t = powf(w[1],-all.power_t);
  if(all.normalized_updates) {
    float norm = w[all.normalized_idx] * avg_norm;
    t *= powf(norm*norm,-power_t_norm);
  }
  w[0] += update * x * t;
}

inline void specialized_update(vw& all, float x, uint32_t fi, float avg_norm, float update)
{
  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float t = 1.f;
  float inv_norm = 1.f;
  if(all.normalized_updates) inv_norm /= (w[all.normalized_idx] * avg_norm);
  if(all.adaptive) {
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
    __m128 eta = _mm_load_ss(&w[1]);
    eta = _mm_rsqrt_ss(eta);
    _mm_store_ss(&t, eta);
    t *= inv_norm;
#else
    t = InvSqrt(w[1]) * inv_norm;
#endif
  } else {
    t *= inv_norm*inv_norm; //if only using normalized updates but not adaptive, need to divide by feature norm squared
  }
  w[0] += update * x * t;
}



void learn_gd(void* a, example* ec)
{
  vw* all = (vw*)a;
  assert(ec->in_use);
  if (ec->pass != gd_current_pass)
    {
      
      if(all->span_server != "") {
	if(all->adaptive)
	  accumulate_weighted_avg(*all, all->span_server, all->reg);
	else 
	  accumulate_avg(*all, all->span_server, all->reg, 0);	      
      }
      
      if (all->save_per_pass)
	{
	  sync_weights(*all);
	  save_predictor(*all, all->final_regressor_name, gd_current_pass);
	}
      all->eta *= all->eta_decay_rate;
      
      gd_current_pass = ec->pass;
    }
  
  if (!command_example(*all, ec))
    {
      predict(*all,ec);
      if (ec->eta_round != 0.)
	{
          if(all->power_t == 0.5)
            inline_train(*all, ec, ec->eta_round);
            //generic_train<specialized_update>(*all,ec,ec->eta_round);
          else
            general_train(*all, ec, ec->eta_round, all->power_t);
            //generic_train<general_update>(*all,ec,ec->eta_round);

	  if (all->sd->contraction < 1e-10)  // updating weights now to avoid numerical instability
	    sync_weights(*all);
	  
	}
    }
}

void finish_gd(void* a)
{
  vw* all = (vw*)a;
  sync_weights(*all);
  if(all->span_server != "") {
    if(all->adaptive)
      accumulate_weighted_avg(*all, all->span_server, all->reg);
    else
      accumulate_avg(*all, all->span_server, all->reg, 0);
  }
}

void sync_weights(vw& all) {
  if (all.sd->gravity == 0. && all.sd->contraction == 1.)  // to avoid unnecessary weight synchronization
    return;
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  for(uint32_t i = 0; i < length && all.reg_mode; i++)
    all.reg.weight_vectors[stride*i] = trunc_weight(all.reg.weight_vectors[stride*i], (float)all.sd->gravity) * (float)all.sd->contraction;
  all.sd->gravity = 0.;
  all.sd->contraction = 1.;
}

bool command_example(vw& all, example* ec) {
  if (ec->indices.size() > 1)
    return false;

  if (ec->tag.size() >= 4 && !strncmp((const char*) ec->tag.begin, "save", 4))
    {//save state
      string final_regressor_name = all.final_regressor_name;

      if ((ec->tag).size() >= 6 && (ec->tag)[4] == '_')
	final_regressor_name = string(ec->tag.begin+5, (ec->tag).size()-5);

      if (!all.quiet)
	cerr << "saving regressor to " << final_regressor_name << endl;
      save_predictor(all, final_regressor_name, 0);

      return true;
    }
  return false;
}

float finalize_prediction(vw& all, float ret) 
{
  if ( nanpattern(ret))
    {
      cout << "you have a NAN!!!!!" << endl;
      return 0.;
    }
  if ( ret > all.sd->max_label )
    return (float)all.sd->max_label;
  if (ret < all.sd->min_label)
    return (float)all.sd->min_label;
  return ret;
}

void finish_example(vw& all, example* ec)
{
  return_simple_example(all, ec);
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

void print_audit_quad(vw& all, weight* weights, audit_data& page_feature, v_array<audit_data> &offer_features, vector<string_value>& features)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;

  for (audit_data* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      ostringstream tempstream;
      tempstream << page_feature.space << '^' << page_feature.feature << '^' 
		 << ele->space << '^' << ele->feature << ':' << (((halfhash + ele->weight_index)/all.stride) & all.parse_mask)
		 << ':' << ele->x*page_feature.x
		 << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & all.weight_mask], (float)all.sd->gravity) * (float)all.sd->contraction;
      string_value sv = {weights[ele->weight_index & all.weight_mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

void print_quad(vw& all, weight* weights, feature& page_feature, v_array<feature> &offer_features, vector<string_value>& features)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      ostringstream tempstream;
      tempstream << (((halfhash + ele->weight_index)/all.stride) & all.parse_mask) 
		 << ':' << (ele->x*page_feature.x)
		 << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & all.weight_mask], (float)all.sd->gravity) * (float)all.sd->contraction;
      string_value sv = {weights[ele->weight_index & all.weight_mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

void print_audit_cubic(vw& all, weight* weights, audit_data& f0, audit_data& f1, v_array<audit_data> &cross_features, vector<string_value>& features)
{
  size_t halfhash = cubic_constant2 * (cubic_constant * f0.weight_index + f1.weight_index);

  for (audit_data* ele = cross_features.begin; ele != cross_features.end; ele++)
    {
      ostringstream tempstream;
      tempstream << f0.space << '^' << f0.feature << '^' 
                 << f1.space << '^' << f1.feature << '^' 
		 << ele->space << '^' << ele->feature << ':' << (((halfhash + ele->weight_index)/all.stride) & all.parse_mask)
		 << ':' << ele->x*f0.x*f1.x
		 << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & all.weight_mask], (float)all.sd->gravity) * all.sd->contraction;
      string_value sv = {weights[ele->weight_index & all.weight_mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

 void print_cubic(vw& all, weight* weights, feature& f0, feature& f1, v_array<feature> &cross_features, vector<string_value>& features)
{
  size_t halfhash = cubic_constant2 * (cubic_constant * f0.weight_index + f1.weight_index);

  for (feature* ele = cross_features.begin; ele != cross_features.end; ele++)
    {
      ostringstream tempstream;
      tempstream << (((halfhash + ele->weight_index)/all.stride) & all.parse_mask) 
		 << ':' << (ele->x*f0.x*f1.x)
		 << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & all.weight_mask], (float)all.sd->gravity) * (float)all.sd->contraction;
      string_value sv = {weights[ele->weight_index & all.weight_mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

void print_features(vw& all, example* &ec)
{
  weight* weights = all.reg.weight_vectors;
  size_t stride = all.stride;

  if (all.lda > 0)
    {
      size_t count = 0;
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
	count += ec->audit_features[*i].size() + ec->atomics[*i].size();
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
	for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	  {
	    cout << '\t' << f->space << '^' << f->feature << ':' << (f->weight_index/all.stride & all.parse_mask) << ':' << f->x;
	    for (size_t k = 0; k < all.lda; k++)
	      cout << ':' << weights[(f->weight_index+k) & all.weight_mask];
	  }
      cout << " total of " << count << " features." << endl;
    }
  else
    {
      vector<string_value> features;

      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
	if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
	  for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	    {
	      ostringstream tempstream;
	      tempstream << f->space << '^' << f->feature << ':' << (f->weight_index/stride & all.parse_mask) << ':' << f->x;
	      tempstream  << ':' << trunc_weight(weights[f->weight_index & all.weight_mask], (float)all.sd->gravity) * (float)all.sd->contraction;
	      if(all.adaptive)
		tempstream << '@' << weights[(f->weight_index+1) & all.weight_mask];
	      string_value sv = {weights[f->weight_index & all.weight_mask]*f->x, tempstream.str()};
	      features.push_back(sv);
	    }
	else
	  for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	    {
	      ostringstream tempstream;
	      if ( f->weight_index == ((constant*stride)&all.weight_mask))
		tempstream << "Constant:";
	      tempstream << (f->weight_index/stride & all.parse_mask) << ':' << f->x;
	      tempstream << ':' << trunc_weight(weights[f->weight_index & all.weight_mask], (float)all.sd->gravity) * (float)all.sd->contraction;
	      if(all.adaptive)
		tempstream << '@' << weights[(f->weight_index+1) & all.weight_mask];
	      string_value sv = {weights[f->weight_index & all.weight_mask]*f->x, tempstream.str()};
	      features.push_back(sv);
	    }
      for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
	if (ec->audit_features[(int)(*i)[0]].begin != ec->audit_features[(int)(*i)[0]].end)
	  for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f != ec->audit_features[(int)(*i)[0]].end; f++)
	    print_audit_quad(all, weights, *f, ec->audit_features[(int)(*i)[1]], features);
	else
	  for (feature* f = ec->atomics[(int)(*i)[0]].begin; f != ec->atomics[(int)(*i)[0]].end; f++)
	    print_quad(all, weights, *f, ec->atomics[(int)(*i)[1]], features);      

      for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) 
	if (ec->audit_features[(int)(*i)[0]].begin != ec->audit_features[(int)(*i)[0]].end) {
	  for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f != ec->audit_features[(int)(*i)[0]].end; f++)
            for (audit_data* f2 = ec->audit_features[(int)(*i)[1]].begin; f2 != ec->audit_features[(int)(*i)[1]].end; f2++)
              print_audit_cubic(all, weights, *f, *f2, ec->audit_features[(int)(*i)[2]], features);
        } else {
	  for (feature* f = ec->atomics[(int)(*i)[0]].begin; f != ec->atomics[(int)(*i)[0]].end; f++)
            for (feature* f2 = ec->atomics[(int)(*i)[1]].begin; f2 != ec->atomics[(int)(*i)[1]].end; f2++)
              print_cubic(all, weights, *f, *f2, ec->atomics[(int)(*i)[2]], features);
        }

      sort(features.begin(),features.end());

      for (vector<string_value>::iterator sv = features.begin(); sv!= features.end(); sv++)
	cout << '\t' << (*sv).s;
      cout << endl;
    }
}

void print_audit_features(vw& all, example* ec)
{
  fflush(stdout);
  print_result(fileno(stdout),ec->final_prediction,-1,ec->tag);
  fflush(stdout);
  print_features(all, ec);
}

template <void (*T)(vw&,float,uint32_t,float,float&,float&)>
void norm_add(vw& all, feature* begin, feature* end, float g, float& norm, float& norm_x, uint32_t offset=0)
{
  for (feature* f = begin; f!= end; f++)
    T(all, f->x, f->weight_index + offset, g, norm, norm_x);
}

template <void (*T)(vw&,float,uint32_t,float,float&,float&)>
void norm_add_quad(vw& all, feature& f0, v_array<feature> &cross_features, float g, float& norm, float& norm_x, uint32_t offset=0)
{
  size_t halfhash = quadratic_constant * (f0.weight_index + offset);
  float norm_new = 0.f;
  float norm_x_new = 0.f;
  norm_add<T>(all, cross_features.begin, cross_features.end, g * f0.x * f0.x, norm_new, norm_x_new, halfhash + offset);
  norm   += norm_new   * f0.x * f0.x;
  norm_x += norm_x_new * f0.x * f0.x;
}

template <void (*T)(vw&,float,uint32_t,float,float&,float&)>
void norm_add_cubic(vw& all, feature& f0, feature& f1, v_array<feature> &cross_features, float g, float& norm, float& norm_x, uint32_t offset=0)
{
  size_t halfhash = cubic_constant2 * (cubic_constant * (f0.weight_index + offset) + f1.weight_index + offset);
  float norm_new = 0.f;
  float norm_x_new = 0.f;
  norm_add<T>(all, cross_features.begin, cross_features.end, g * f0.x * f0.x * f1.x * f1.x, norm_new, norm_x_new, halfhash + offset);
  norm   += norm_new   * f0.x * f0.x * f1.x * f1.x;
  norm_x += norm_x_new * f0.x * f0.x * f1.x * f1.x;
}

inline void simple_norm_compute(vw& all, float x, uint32_t fi, float g, float& norm, float& norm_x) {
  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float x2 = x * x;
  float t = 1.f;
  float inv_norm = 1.f;
  float inv_norm2 = 1.f;
  if(all.normalized_updates) {
    inv_norm /= w[all.normalized_idx];
    inv_norm2 = inv_norm*inv_norm;
    norm_x += x2 * inv_norm2;
  }
  if(all.adaptive){
    w[1] += g * x2;
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
    __m128 eta = _mm_load_ss(&w[1]);
    eta = _mm_rsqrt_ss(eta);
    _mm_store_ss(&t, eta);
    t *= inv_norm;
#else
    t = InvSqrt(w[1]) * inv_norm;
#endif
  } else {
    t *= inv_norm2; //if only using normalized but not adaptive, we're dividing update by feature norm squared
  }
  norm += x2 * t;
}

inline void powert_norm_compute(vw& all, float x, uint32_t fi, float g, float& norm, float& norm_x) {
  float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);

  weight* w = &all.reg.weight_vectors[fi & all.weight_mask];
  float x2 = x * x;
  float t = 1.f;
  if(all.adaptive){
    w[1] += g * x2;
    t = powf(w[1], -all.power_t);
  }
  if(all.normalized_updates) {
    float range2 = w[all.normalized_idx] * w[all.normalized_idx];
    t *= powf(range2, -power_t_norm);
    norm_x += x2 / range2;
  }
  norm += x2 * t;
}

template <void (*T)(vw&,float,uint32_t,float,float&,float&)>
float compute_norm(vw& all, example* &ec, uint32_t offset=0)
{//We must traverse the features in _precisely_ the same order as during training.
  label_data* ld = (label_data*)ec->ld;
  float g = all.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  if (g==0) return 1.;

  float norm = 0.;
  float norm_x = 0.;

  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
    norm_add<T>(all, ec->atomics[*i].begin, ec->atomics[*i].end, g, norm, norm_x, offset);

  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end(); i++)
    if (ec->atomics[(int)(*i)[0]].size() > 0)
      for (feature* f0 = ec->atomics[(int)(*i)[0]].begin; f0 != ec->atomics[(int)(*i)[0]].end; f0++)
        norm_add_quad<T>(all, *f0, ec->atomics[(int)(*i)[1]], g, norm, norm_x, offset);

  for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) 
    if ((ec->atomics[(int)(*i)[0]].size() > 0) && (ec->atomics[(int)(*i)[1]].size() > 0) && (ec->atomics[(int)(*i)[2]].size() > 0))
      for (feature* f0 = ec->atomics[(int)(*i)[0]].begin; f0 != ec->atomics[(int)(*i)[0]].end; f0++)
        for (feature* f1 = ec->atomics[(int)(*i)[1]].begin; f1 != ec->atomics[(int)(*i)[1]].end; f1++)
          norm_add_cubic<T>(all, *f0, *f1, ec->atomics[(int)(*i)[2]], g, norm, norm_x, offset);
  
  if(all.normalized_updates) {
    float total_weight = 0;
    if(all.active)
      total_weight = (float)all.sd->weighted_unlabeled_examples;
    else
      total_weight = ec->example_t;

    all.normalized_sum_norm_x += ld->weight * norm_x;
    float avg_sq_norm = all.normalized_sum_norm_x / total_weight;

    if(all.power_t == 0.5) {
      if(all.adaptive) norm /= sqrt(avg_sq_norm);
      else norm /= avg_sq_norm;
    } else {
      float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
      norm *= powf(avg_sq_norm,-power_t_norm);
    }
  }
  
  return norm;
}

void local_predict(vw& all, example* ec)
{
  label_data* ld = (label_data*)ec->ld;

  all.set_minmax(all.sd, ld->label);

  ec->final_prediction = finalize_prediction(all, ec->partial_prediction * (float)all.sd->contraction);

  if(all.active_simulation){
    float k = ec->example_t - ld->weight;
    ec->revert_weight = all.loss->getRevertingWeight(all.sd, ec->final_prediction, all.eta/powf(k,all.power_t));
    float importance = query_decision(all, ec, k);
    if(importance > 0){
      all.sd->queries += 1;
      ld->weight *= importance;
    }
    else //do not query => do not train
      ld->label = FLT_MAX;
  }

  float t;
  if(all.active)
    t = (float)all.sd->weighted_unlabeled_examples;
  else
    t = ec->example_t;

  ec->eta_round = 0;
  if (ld->label != FLT_MAX)
    {
      ec->loss = all.loss->getLoss(all.sd, ec->final_prediction, ld->label) * ld->weight;
      if (all.training && ec->loss > 0.)
	{
	  float eta_t;
	  float norm;
          if(all.adaptive || all.normalized_updates) {
            if(all.power_t == 0.5)
              norm = compute_norm<simple_norm_compute>(all,ec);
            else
              norm = compute_norm<powert_norm_compute>(all,ec);
          }
          else {
            norm = ec->total_sum_feat_sq;  
          }

          eta_t = all.eta * norm * ld->weight;
          if(!all.adaptive) eta_t *= powf(t,-all.power_t);

          float update = 0.f;
          if( all.invariant_updates )
            update = all.loss->getUpdate(ec->final_prediction, ld->label, eta_t, norm);
          else
            update = all.loss->getUnsafeUpdate(ec->final_prediction, ld->label, eta_t, norm);

	  ec->eta_round = (float) (update / all.sd->contraction);

	  if (all.reg_mode && fabs(ec->eta_round) > 1e-8) {
	    double dev1 = all.loss->first_derivative(all.sd, ec->final_prediction, ld->label);
	    double eta_bar = (fabs(dev1) > 1e-8) ? (-ec->eta_round / dev1) : 0.0;
	    if (fabs(dev1) > 1e-8)
	      all.sd->contraction /= (1. + all.l2_lambda * eta_bar * norm);
	    all.sd->gravity += eta_bar * sqrt(norm) * all.l1_lambda;
	  }
	}
    }
  else if(all.active)
    ec->revert_weight = all.loss->getRevertingWeight(all.sd, ec->final_prediction, all.eta/powf(t,all.power_t));

  if (all.audit)
    print_audit_features(all, ec);
}


void one_pf_quad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, float g, example* ec, 
                        bool is_adaptive, bool is_normalized, size_t idx_norm, float avg_norm)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  update *= page_feature.x;

  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
  {
    weight* w=&weights[(halfhash + ele->weight_index) & mask];
    float t = 1.f;
    float inv_norm = 1.f;
    if(is_normalized) inv_norm /= (w[idx_norm] * avg_norm);
    if(is_adaptive) {
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
      __m128 eta = _mm_load_ss(&w[1]);
      eta = _mm_rsqrt_ss(eta);
      _mm_store_ss(&t, eta);
      t *= inv_norm;
#else
      t = InvSqrt(w[1]) * inv_norm;
#endif
    }
    else {
      t *= inv_norm * inv_norm; //if only using normalized updates but not adaptive, need to divide by feature norm squared
    }
    w[0] += update * ele->x * t;
  }
}

void quad_general_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, float g, example* ec, float power_t, 
                         bool is_adaptive, bool is_normalized, size_t idx_norm, float avg_norm, float power_t_norm)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  update *= page_feature.x;
  
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
  {
    weight* w=&weights[(halfhash + ele->weight_index) & mask];
    float t=1.f;
    if(is_adaptive) t = powf(w[1],-power_t);
    if(is_normalized) {
      float norm = w[idx_norm]*avg_norm;
      t *= powf(norm*norm,-power_t_norm); 
    }
    w[0] += update * ele->x * t;
  }
}

void general_train(vw& all, example* &ec, float update, float power_t)
{
  if (fabs(update) == 0.)
    return;
  
  size_t mask = all.weight_mask;
  label_data* ld = (label_data*)ec->ld;
  weight* weights = all.reg.weight_vectors;

  size_t idx_norm = all.normalized_idx;
  bool is_adaptive = all.adaptive;
  bool is_normalized = all.normalized_updates;
  float total_weight = 0.f;
  if(all.active)
    total_weight = (float)all.sd->weighted_unlabeled_examples;
  else
    total_weight = ec->example_t;

  float avg_norm = all.normalized_sum_norm_x / total_weight;
 
  float power_t_norm = 1.f;
  if(is_adaptive) power_t_norm -= power_t;

  float g = all.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
  {
    feature* f = ec->atomics[*i].begin;
    for (; f != ec->atomics[*i].end; f++)
    {
      weight* w = &weights[f->weight_index & mask];
      float t = 1.f;
      if(is_adaptive) t = powf(w[1],-power_t);
      if(is_normalized) {
        float norm = w[idx_norm]*avg_norm;
        t *= powf(norm*norm,-power_t_norm);
      }
      w[0] += update * f->x * t;
    }
  }
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
  {
    if (ec->atomics[(int)(*i)[0]].size() > 0)
      {
        v_array<feature> temp = ec->atomics[(int)(*i)[0]];
        for (; temp.begin != temp.end; temp.begin++)
          quad_general_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, update, g, ec, power_t, is_adaptive, is_normalized, idx_norm, avg_norm, power_t_norm);
      } 
  }
}

void inline_train(vw& all, example* &ec, float update)
{
  if (fabs(update) == 0.)
    return;

  size_t mask = all.weight_mask;
  label_data* ld = (label_data*)ec->ld;
  weight* weights = all.reg.weight_vectors;

  size_t idx_norm = all.normalized_idx;
  bool is_adaptive = all.adaptive;
  bool is_normalized = all.normalized_updates;

  float total_weight;
  if(all.active)
    total_weight = (float)all.sd->weighted_unlabeled_examples;
  else
    total_weight = ec->example_t;

  float avg_norm = sqrt(all.normalized_sum_norm_x / total_weight);

  float g = all.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
  {
    feature* f = ec->atomics[*i].begin;
    for (; f != ec->atomics[*i].end; f++)
    {
      weight* w = &weights[f->weight_index & mask];
      float t = 1.f;
      float inv_norm = 1.f;
      if( is_normalized ) inv_norm /= (w[idx_norm] * avg_norm);
      if(is_adaptive) {
#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
        __m128 eta = _mm_load_ss(&w[1]);
        eta = _mm_rsqrt_ss(eta);
        _mm_store_ss(&t, eta);
        t *= inv_norm;
#else
        t = InvSqrt(w[1]) * inv_norm;
#endif
      }
      else {
        t *= inv_norm*inv_norm; //if only using normalized updates but not adaptive, need to divide by feature norm squared
      }
      w[0] += update * f->x * t;
    }
  }
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
  {
    if (ec->atomics[(int)(*i)[0]].size() > 0)
      {
        v_array<feature> temp = ec->atomics[(int)(*i)[0]];
        for (; temp.begin != temp.end; temp.begin++)
          one_pf_quad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, update, g, ec, is_adaptive, is_normalized, idx_norm, avg_norm);
      } 
  }
}

void predict(vw& all, example* ex)
{
  label_data* ld = (label_data*)ex->ld;
  float prediction;
  if (all.training && all.normalized_updates && ld->label != FLT_MAX && ld->weight > 0) {
    if( all.power_t == 0.5 ) {
      if (all.reg_mode % 2)
        prediction = inline_predict<vec_add_trunc_rescale>(all, ex);
      else
        prediction = inline_predict<vec_add_rescale>(all, ex);
    }
    else {
      if (all.reg_mode % 2)
        prediction = inline_predict<vec_add_trunc_rescale_general>(all, ex);
      else
        prediction = inline_predict<vec_add_rescale_general>(all, ex);
    }
  }
  else {
    if (all.reg_mode % 2)
      prediction = inline_predict<vec_add_trunc>(all, ex);
    else
      prediction = inline_predict<vec_add>(all, ex);
  }

  ex->partial_prediction += prediction;

  local_predict(all, ex);
  ex->done = true;
}

void drive_gd(void* in)
{
  vw* all = (vw*)in;
  example* ec = NULL;
  
  while ( true )
    {
      if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
	{
	  learn_gd(all, ec);
	  finish_example(*all, ec);
	}
      else if (parser_done(all->p))
	{
	  finish_gd(all);
	  return;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
}
