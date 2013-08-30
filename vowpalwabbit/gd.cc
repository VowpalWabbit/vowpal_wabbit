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
#include "accumulate.h"
#include "learner.h"
#include "vw.h"

using namespace std;

namespace GD
{
  struct gd{
    size_t current_pass;
    bool active;
    bool active_simulation;
    float normalized_sum_norm_x;
    bool feature_mask_off;
    size_t no_win_counter;
    size_t early_stop_thres;

    vw* all;
  };
  void predict(vw& all, gd& g, example* ex);
  void sync_weights(vw& all);
  
  struct train_data {
    float avg_norm;
    float update;
  };

  template <void (*T)(vw&, void*, float, uint32_t)>
  void generic_train(vw& all, example* &ec, float update, bool sqrt_norm)
  {
    if (fabs(update) == 0.)
      return;
    
    float total_weight = 0.f;
    if(all.active)
      total_weight = (float)all.sd->weighted_unlabeled_examples;
    else
      total_weight = ec->example_t;

    if(!all.holdout_set_off)
      total_weight -= all.sd->weighted_holdout_examples; //exclude weights from test_only examples   
    
    float avg_norm = all.normalized_sum_norm_x / total_weight;
    if (sqrt_norm) avg_norm = sqrt(avg_norm);
    
    train_data d = {avg_norm, update};
    
    foreach_feature<T>(all, ec, &d);
  }

float InvSqrt(float x){
  float xhalf = 0.5f * x;
  int i = *(int*)&x; // store floating-point bits in integer
  i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
  x = *(float*)&i; // convert new bits into float
  x = x*(1.5f - xhalf*x*x); // One round of Newton's method
  return x;
}

template<bool feature_mask_off>
inline void general_update(vw& all, void* dat, float x, uint32_t fi)
{
  if(feature_mask_off || all.reg.weight_vector[(fi & all.reg.weight_mask)+all.feature_mask_idx]==1.){
    train_data* s = (train_data*)dat;

    weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
    float t = 1.f;
    if(all.adaptive) t = powf(w[1],-all.power_t);
    if(all.normalized_updates) {
      float norm = w[all.normalized_idx] * s->avg_norm;
      float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
      t *= powf(norm*norm,-power_t_norm);
    }
    w[0] += s->update * x * t;
  }
}

template<bool adaptive, bool normalized, bool feature_mask_off>
inline void specialized_update(vw& all, void* dat, float x, uint32_t fi)
{
  if(feature_mask_off || all.reg.weight_vector[(fi & all.reg.weight_mask)+all.feature_mask_idx]==1.){
    train_data* s = (train_data*)dat;

    weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
    float t = 1.f;
    float inv_norm = 1.f;
    if(normalized) inv_norm /= (w[all.normalized_idx] * s->avg_norm);
    if(adaptive) {
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
    w[0] += s->update * x * t;
  }
}

void learn(void* d, example* ec)
{
  gd* g = (gd*)d;
  vw* all = g->all;

  assert(ec->in_use);
  if (ec->end_pass)
    { 
      sync_weights(*all);
      if(all->span_server != "") {
	if(all->adaptive)
	  accumulate_weighted_avg(*all, all->span_server, all->reg);
	else 
	  accumulate_avg(*all, all->span_server, all->reg, 0);	      
      }
      
      all->eta *= all->eta_decay_rate;
      if (all->save_per_pass)
	save_predictor(*all, all->final_regressor_name, all->current_pass);   
      
      all->current_pass++;

      if(!all->holdout_set_off)
      {
        if(summarize_holdout_set(*all, g->no_win_counter))
          finalize_regressor(*all, all->final_regressor_name); 
        if(g->early_stop_thres == g->no_win_counter)
          all-> early_terminate = true;
      }   
    }
  
  if (!command_example(all, ec))
    { 
      predict(*all,*g,ec);

      if (all->holdout_set_off || !ec->test_only)
      {
      if (ec->eta_round != 0.)
	{ 
          if(all->power_t == 0.5) { 
            if (all->adaptive) {
              if (all->normalized_updates){ 
                if (g->feature_mask_off) 
                  generic_train<specialized_update<true, true, true> >
                    (*all,ec,ec->eta_round,true);
                else
                  generic_train<specialized_update<true, true, false> >
                    (*all,ec,ec->eta_round,true);
              }
              else {
                if (g->feature_mask_off) 
                  generic_train<specialized_update<true, false, true> >
                    (*all,ec,ec->eta_round,true);
                else
                  generic_train<specialized_update<true, false, false> >
                    (*all,ec,ec->eta_round,true);
              }
            }              
            else { //for adaptive 
              if (all->normalized_updates){ 
                if (g->feature_mask_off) 
                  generic_train<specialized_update<false, true, true> >
                    (*all,ec,ec->eta_round,true);
                else
                  generic_train<specialized_update<false, true, false> >
                    (*all,ec,ec->eta_round,true);
              }
              else {
                if (g->feature_mask_off) 
                  generic_train<specialized_update<false, false, true> >
                    (*all,ec,ec->eta_round,true);
                else
                  generic_train<specialized_update<false, false, false> >
                    (*all,ec,ec->eta_round,true);
              }  
            }
          }//end of power_t
          else{
            if (g->feature_mask_off)
              generic_train<general_update<true> >(*all,ec,ec->eta_round,false);
            else
              generic_train<general_update<false> >(*all,ec,ec->eta_round,false);
          }  
	  if (all->sd->contraction < 1e-10)  // updating weights now to avoid numerical instability
	    sync_weights(*all);
	}
      }
    }
}
  void finish(void* d)
{
  gd* g = (gd*)d;
  free(g);
}

void sync_weights(vw& all) {
  if (all.sd->gravity == 0. && all.sd->contraction == 1.)  // to avoid unnecessary weight synchronization
    return;
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.reg.stride;
  for(uint32_t i = 0; i < length && all.reg_mode; i++)
    all.reg.weight_vector[stride*i] = trunc_weight(all.reg.weight_vector[stride*i], (float)all.sd->gravity) * (float)all.sd->contraction;
  all.sd->gravity = 0.;
  all.sd->contraction = 1.;
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

void audit_feature(vw& all, feature* f, audit_data* a, vector<string_value>& results, string prepend, string& ns_pre, size_t offset = 0)
{ 
  ostringstream tempstream;
  size_t index = (f->weight_index + offset) & all.reg.weight_mask;
  weight* weights = all.reg.weight_vector;
  size_t stride = all.reg.stride;
  
  if(all.audit) tempstream << prepend;
  
  string tmp = "";
  
  if (a != NULL){
    tmp += a->space;
    tmp += '^';
    tmp += a->feature; 
  }
 
  if (a != NULL && all.audit){
    tempstream << tmp << ':';
  }
  else 	if ( index == (((constant * stride + offset)&all.reg.weight_mask)) && all.audit){
    tempstream << "Constant:";
  }  
  if(all.audit){
    tempstream << (index/stride & all.parse_mask) << ':' << f->x;
    tempstream  << ':' << trunc_weight(weights[index], (float)all.sd->gravity) * (float)all.sd->contraction;
  }
  if(all.current_pass == 0 && all.inv_hash_regressor_name != ""){ //for invert_hash
    if ( index == ((constant * stride + offset )& all.reg.weight_mask))
      tmp = "Constant";

    ostringstream convert;
    convert << (index/stride & all.parse_mask);
    tmp = ns_pre + tmp + ":"+ convert.str();
    
    if(!all.name_index_map.count(tmp)){
      all.name_index_map.insert(std::map< std::string, size_t>::value_type(tmp, (index/stride & all.parse_mask)));
    }
  }

  if(all.adaptive && all.audit)
    tempstream << '@' << weights[index+1];
  string_value sv = {weights[index]*f->x, tempstream.str()};
  results.push_back(sv);
}

void audit_features(vw& all, v_array<feature>& fs, v_array<audit_data>& as, vector<string_value>& results, string prepend, string& ns_pre, size_t offset = 0)
{
  for (size_t j = 0; j< fs.size(); j++)
    if (as.begin != as.end)
      audit_feature(all, & fs[j], & as[j], results, prepend, ns_pre, offset);
    else
      audit_feature(all, & fs[j], NULL, results, prepend, ns_pre, offset);
}

void audit_quad(vw& all, feature& left_feature, audit_data* left_audit, v_array<feature> &right_features, v_array<audit_data> &audit_right, vector<string_value>& results, string& ns_pre, uint32_t offset = 0)
{
  size_t halfhash = quadratic_constant * (left_feature.weight_index + offset);

  ostringstream tempstream;
  if (audit_right.size() != 0 && left_audit && all.audit)
    tempstream << left_audit->space << '^' << left_audit->feature << '^';
  string prepend = tempstream.str();

  if(all.current_pass == 0 && audit_right.size() != 0 && left_audit)//for invert_hash
  {
    ns_pre = left_audit->space; 
    ns_pre = ns_pre + '^' + left_audit->feature + '^';
  }
 
  audit_features(all, right_features, audit_right, results, prepend, ns_pre, halfhash + offset);
}

void audit_triple(vw& all, feature& f0, audit_data* f0_audit, feature& f1, audit_data* f1_audit, 
		  v_array<feature> &right_features, v_array<audit_data> &audit_right, vector<string_value>& results, string& ns_pre, uint32_t offset = 0)
{
  size_t halfhash = cubic_constant2 * (cubic_constant * (f0.weight_index + offset) + f1.weight_index + offset);

  ostringstream tempstream;
  if (audit_right.size() > 0 && f0_audit && f1_audit && all.audit)
    tempstream << f0_audit->space << '^' << f0_audit->feature << '^' 
	       << f1_audit->space << '^' << f1_audit->feature << '^';
  string prepend = tempstream.str();

  if(all.current_pass == 0 && audit_right.size() != 0 && f0_audit && f1_audit)//for invert_hash
  {
    ns_pre = f0_audit->space;
    ns_pre = ns_pre + '^' + f0_audit->feature + '^' + f1_audit->space + '^' + f1_audit->feature + '^';
  }
  audit_features(all, right_features, audit_right, results, prepend, ns_pre, halfhash + offset);  
}

void print_features(vw& all, example* &ec)
{
  weight* weights = all.reg.weight_vector;
  
  if (all.lda > 0)
    {
      size_t count = 0;
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
	count += ec->audit_features[*i].size() + ec->atomics[*i].size();
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
	for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	  {
	    cout << '\t' << f->space << '^' << f->feature << ':' << (f->weight_index/all.reg.stride & all.parse_mask) << ':' << f->x;
	    for (size_t k = 0; k < all.lda; k++)
	      cout << ':' << weights[(f->weight_index+k) & all.reg.weight_mask];
	  }
      cout << " total of " << count << " features." << endl;
    }
  else
    {
      vector<string_value> features;
      string empty;
      string ns_pre;
      
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++){ 
        ns_pre = "";
	audit_features(all, ec->atomics[*i], ec->audit_features[*i], features, empty, ns_pre, ec->ft_offset);
        ns_pre = "";
      }
      for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
	{
	  int fst = (*i)[0];
	  int snd = (*i)[1];
	  for (size_t j = 0; j < ec->atomics[fst].size(); j++)
	    {
	      audit_data* a = NULL;
	      if (ec->audit_features[fst].size() > 0)
		a = & ec->audit_features[fst][j];
	      audit_quad(all, ec->atomics[fst][j], a, ec->atomics[snd], ec->audit_features[snd], features, ns_pre);
	    }
	}

      for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) 
	{
	  int fst = (*i)[0];
	  int snd = (*i)[1];
	  int trd = (*i)[2];
	  for (size_t j = 0; j < ec->atomics[fst].size(); j++)
	    {
	      audit_data* a1 = NULL;
	      if (ec->audit_features[fst].size() > 0)
		a1 = & ec->audit_features[fst][j];
	      for (size_t k = 0; k < ec->atomics[snd].size(); k++)
		{
		  audit_data* a2 = NULL;
		  if (ec->audit_features[snd].size() > 0)
		    a2 = & ec->audit_features[snd][k];
		  audit_triple(all, ec->atomics[fst][j], a1, ec->atomics[snd][k], a2, ec->atomics[trd], ec->audit_features[trd], features, ns_pre);
		}
	    }
	}

      sort(features.begin(),features.end());
      if(all.audit){ 
        for (vector<string_value>::iterator sv = features.begin(); sv!= features.end(); sv++)
	  cout << '\t' << (*sv).s;
        cout << endl;
      }
    }
}

void print_audit_features(vw& all, example* ec)
{
  if(all.audit)
    print_result(all.stdout_fileno,ec->final_prediction,-1,ec->tag);
  fflush(stdout);
  print_features(all, ec);
}

  struct norm_data {
    float g;
    float norm;
    float norm_x;
  };

template<bool adaptive, bool normalized, bool feature_mask_off>
inline void simple_norm_compute(vw& all, void* v, float x, uint32_t fi) {

  if(feature_mask_off || all.reg.weight_vector[(fi & all.reg.weight_mask)+all.feature_mask_idx]==1.){
    norm_data* nd=(norm_data*)v;
    weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
    float x2 = x * x;
    float t = 1.f;
    float inv_norm = 1.f;
    float inv_norm2 = 1.f;
    if(normalized) {
      inv_norm /= w[all.normalized_idx];
      inv_norm2 = inv_norm*inv_norm;
      nd->norm_x += x2 * inv_norm2;
    }
    if(adaptive){
      w[1] += nd->g * x2;

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
    nd->norm += x2 * t;
  }
}

template<bool feature_mask_off>
inline void powert_norm_compute(vw& all, void* v, float x, uint32_t fi) {
  if(feature_mask_off || all.reg.weight_vector[(fi & all.reg.weight_mask)+all.feature_mask_idx]==1.){
    norm_data* nd=(norm_data*)v;
    float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);

    weight* w = &all.reg.weight_vector[fi & all.reg.weight_mask];
    float x2 = x * x;
    float t = 1.f;
    if(all.adaptive){
      w[1] += nd->g * x2;
      t = powf(w[1], -all.power_t);
    }
    if(all.normalized_updates) {
      float range2 = w[all.normalized_idx] * w[all.normalized_idx];
      t *= powf(range2, -power_t_norm);
      nd->norm_x += x2 / range2;
    }
    nd->norm += x2 * t;
  }
}

  template <void (*T)(vw&,void*,float,uint32_t)>
float compute_norm(vw& all, example* &ec)
{//We must traverse the features in _precisely_ the same order as during training.
  label_data* ld = (label_data*)ec->ld;
  float g = all.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  if (g==0) return 1.;

  norm_data nd = {g, 0., 0.};

  foreach_feature<T>(all, ec, &nd);
  
  if(all.normalized_updates) {
    float total_weight = 0;
    if(all.active)
      total_weight = (float)all.sd->weighted_unlabeled_examples;
    else
      total_weight = ec->example_t;

    if(!all.holdout_set_off)
      total_weight -= all.sd->weighted_holdout_examples; //exclude weights from test_only examples   
    
    all.normalized_sum_norm_x += ld->weight * nd.norm_x;
    
    float avg_sq_norm = all.normalized_sum_norm_x / total_weight;
    if(all.power_t == 0.5) {
      if(all.adaptive) nd.norm /= sqrt(avg_sq_norm);
      else nd.norm /= avg_sq_norm;
    } else {
      float power_t_norm = 1.f - (all.adaptive ? all.power_t : 0.f);
      nd.norm *= powf(avg_sq_norm,-power_t_norm);
    }
  }
  
  return nd.norm;
}

  void local_predict(vw& all, gd& g, example* ec)
{
  label_data* ld = (label_data*)ec->ld;

  all.set_minmax(all.sd, ld->label);

  ec->final_prediction = finalize_prediction(all, ec->partial_prediction * (float)all.sd->contraction);
  
  if(g.active_simulation){
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
    t = ec->example_t - all.sd->weighted_holdout_examples;

  ec->eta_round = 0;

  ec->loss = all.loss->getLoss(all.sd, ec->final_prediction, ld->label) * ld->weight;

  if (ld->label != FLT_MAX && !ec->test_only)
    {
      if (all.training && ec->loss > 0.)
        {
	  float eta_t;
	  float norm;
          if(all.adaptive || all.normalized_updates) {
            if(all.power_t == 0.5) {
                if (all.adaptive && all.normalized_updates){
                  if (g.feature_mask_off)
                    norm = compute_norm<simple_norm_compute<true, true, true> >(all,ec);
                  else
                    norm = compute_norm<simple_norm_compute<true, true, false> >(all,ec);
                }
                else if (all.adaptive){
                  if (g.feature_mask_off)
                    norm = compute_norm<simple_norm_compute<true, false, true> >(all,ec);
                  else
                    norm = compute_norm<simple_norm_compute<true, false, false> >(all,ec);
                } 
                else{ 
                   if (g.feature_mask_off)
                    norm = compute_norm<simple_norm_compute<false, true, true> >(all,ec);
                  else
                    norm = compute_norm<simple_norm_compute<false, true, false> >(all,ec);
                }
            }
            else{
              if(g.feature_mask_off)  
                norm = compute_norm<powert_norm_compute<true> >(all,ec);
              else
                norm = compute_norm<powert_norm_compute<false> >(all,ec);
            }
          }
          else 
            norm = ec->total_sum_feat_sq;  

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

  if ((all.audit && !ec->test_only) || all.hash_inv)
    print_audit_features(all, ec);
}

  void predict(vw& all, gd& g, example* ex)
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

  ex->partial_prediction = prediction;

  local_predict(all, g, ex);
  ex->done = true;
}

void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text)
{
  uint32_t length = 1 << all.num_bits;
  uint32_t stride = all.reg.stride;
  int c = 0;
  uint32_t i = 0;
  size_t brw = 1;

  if(all.print_invert){ //write readable model with feature names           
    weight* v;
    char buff[512];
    int text_len; 
    typedef std::map< std::string, size_t> str_int_map;  
        
    for(str_int_map::iterator it = all.name_index_map.begin(); it != all.name_index_map.end(); ++it){              
      v = &(all.reg.weight_vector[stride*(it->second)]);
      if(*v != 0.){
        text_len = sprintf(buff, "%s", (char*)it->first.c_str());
        brw = bin_text_write_fixed(model_file, (char*)it->first.c_str(), sizeof(*it->first.c_str()),
					 buff, text_len, true);
        text_len = sprintf(buff, ":%f\n", *v);
        brw+= bin_text_write_fixed(model_file,(char *)v, sizeof (*v),
					 buff, text_len, true);
      }	
    }
    return;
  } 

  do 
    {
      brw = 1;
      weight* v;
      if (read)
	{
	  c++;
	  brw = bin_read_fixed(model_file, (char*)&i, sizeof(i),"");
	  if (brw > 0)
	    {
	      assert (i< length);		
	      v = &(all.reg.weight_vector[stride*i]);
	      brw += bin_read_fixed(model_file, (char*)v, sizeof(*v), "");
	    }
	}
      else// write binary or text
	{
	  v = &(all.reg.weight_vector[stride*i]);
	  if (*v != 0.)
	    {
	      c++;
	      char buff[512];
	      int text_len;

	      text_len = sprintf(buff, "%d", i);
	      brw = bin_text_write_fixed(model_file,(char *)&i, sizeof (i),
					 buff, text_len, text);
	      
              text_len = sprintf(buff, ":%f\n", *v);
	      brw+= bin_text_write_fixed(model_file,(char *)v, sizeof (*v),
					 buff, text_len, text);
	    }
	}
 
      if (!read)
	i++;
    }
  while ((!read && i < length) || (read && brw >0));  
}

void save_load_online_state(vw& all, io_buf& model_file, bool read, bool text)
{
  char buff[512];
  
  uint32_t text_len = sprintf(buff, "initial_t %f\n", all.initial_t);
  bin_text_read_write_fixed(model_file,(char*)&all.initial_t, sizeof(all.initial_t), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "norm normalizer %f\n", all.normalized_sum_norm_x);
  bin_text_read_write_fixed(model_file,(char*)&all.normalized_sum_norm_x, sizeof(all.normalized_sum_norm_x), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "t %f\n", all.sd->t);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->t, sizeof(all.sd->t), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "sum_loss %f\n", all.sd->sum_loss);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->sum_loss, sizeof(all.sd->sum_loss), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "sum_loss_since_last_dump %f\n", all.sd->sum_loss_since_last_dump);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->sum_loss_since_last_dump, sizeof(all.sd->sum_loss_since_last_dump), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "dump_interval %f\n", all.sd->dump_interval);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->dump_interval, sizeof(all.sd->dump_interval), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "min_label %f\n", all.sd->min_label);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->min_label, sizeof(all.sd->min_label), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "max_label %f\n", all.sd->max_label);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->max_label, sizeof(all.sd->max_label), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "weighted_examples %f\n", all.sd->weighted_examples);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->weighted_examples, sizeof(all.sd->weighted_examples), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "weighted_labels %f\n", all.sd->weighted_labels);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->weighted_labels, sizeof(all.sd->weighted_labels), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "weighted_unlabeled_examples %f\n", all.sd->weighted_unlabeled_examples);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->weighted_unlabeled_examples, sizeof(all.sd->weighted_unlabeled_examples), 
			    "", read, 
			    buff, text_len, text);
  
  text_len = sprintf(buff, "example_number %u\n", (uint32_t)all.sd->example_number);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->example_number, sizeof(all.sd->example_number), 
			    "", read, 
			    buff, text_len, text);

  text_len = sprintf(buff, "total_features %u\n", (uint32_t)all.sd->total_features);
  bin_text_read_write_fixed(model_file,(char*)&all.sd->total_features, sizeof(all.sd->total_features), 
			    "", read, 
			    buff, text_len, text);

  uint32_t length = 1 << all.num_bits;
  uint32_t stride = all.reg.stride;
  int c = 0;
  uint32_t i = 0;
  size_t brw = 1;
  do 
    {
      brw = 1;
      weight* v;
      if (read)
	{
	  c++;
	  brw = bin_read_fixed(model_file, (char*)&i, sizeof(i),"");
	  if (brw > 0)
	    {
	      assert (i< length);		
	      v = &(all.reg.weight_vector[stride*i]);
	      if (stride == 2) //either adaptive or normalized
		brw += bin_read_fixed(model_file, (char*)v, sizeof(*v)*2, "");
	      else //adaptive and normalized
		brw += bin_read_fixed(model_file, (char*)v, sizeof(*v)*3, "");	
	    }
	}
      else // write binary or text
	{
	  v = &(all.reg.weight_vector[stride*i]);
	  if (*v != 0.)
	    {
	      c++;
	      char buff[512];
	      int text_len = sprintf(buff, "%d", i);
	      brw = bin_text_write_fixed(model_file,(char *)&i, sizeof (i),
					 buff, text_len, text);
	      
	      if (stride == 2)
		{//either adaptive or normalized
		  text_len = sprintf(buff, ":%f %f\n", *v, *(v+1));
		  brw+= bin_text_write_fixed(model_file,(char *)v, 2*sizeof (*v),
					     buff, text_len, text);
		}
	      else
		{//adaptive and normalized
		  text_len = sprintf(buff, ":%f %f %f\n", *v, *(v+1), *(v+2));
		  brw+= bin_text_write_fixed(model_file,(char *)v, 3*sizeof (*v),
					     buff, text_len, text);
		}
	    }
	}
      if (!read)
	i++;
    }
  while ((!read && i < length) || (read && brw >0));  
}

void save_load(void* data, io_buf& model_file, bool read, bool text)
{
  gd* g = (gd*)data;
  vw* all = g->all;
  if(read)
    {
      initialize_regressor(*all);
      if(all->adaptive && all->initial_t > 0)
	{
	  uint32_t length = 1 << all->num_bits;
	  uint32_t stride = all->reg.stride;
	  for (size_t j = 1; j < stride*length; j+=stride)
	    {
	      all->reg.weight_vector[j] = all->initial_t;   //for adaptive update, we interpret initial_t as previously seeing initial_t fake datapoints, all with squared gradient=1
	      //NOTE: this is not invariant to the scaling of the data (i.e. when combined with normalized). Since scaling the data scales the gradient, this should ideally be 
	      //feature_range*initial_t, or something like that. We could potentially fix this by just adding this base quantity times the current range to the sum of gradients 
	      //stored in memory at each update, and always start sum of gradients to 0, at the price of additional additions and multiplications during the update...
	    }
	}
    }

  if (model_file.files.size() > 0)
    {
      bool resume = all->save_resume;
      char buff[512];
      uint32_t text_len = sprintf(buff, ":%d\n", resume);
      bin_text_read_write_fixed(model_file,(char *)&resume, sizeof (resume),
				"", read,
				buff, text_len, text);
      if (resume)
	save_load_online_state(*all, model_file, read, text);
      else
	save_load_regressor(*all, model_file, read, text);
    }
}

void driver(vw* all, void* data)
{
  example* ec = NULL;
  
  while ( true )
    {
     if(all-> early_terminate)
        {
          all->p->done = true;
          all->final_regressor_name = "";//skip finalize_regressor
          all->text_regressor_name = "";
          all->inv_hash_regressor_name = "";
          return;
        }
     else if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
	{
	  learn(data, ec);
	  return_simple_example(*all, ec);
	}
      else if (parser_done(all->p))
	return;
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
}

learner setup(vw& all, po::variables_map& vm)
{
  gd* g = (gd*)calloc(1, sizeof(gd));
  g->all = &all;
  g->active = all.active;
  g->active_simulation = all.active_simulation;
  g->normalized_sum_norm_x = all.normalized_sum_norm_x;
  g->no_win_counter = 0;
  g->early_stop_thres = 3;

  if(vm.count("feature_mask"))
    g->feature_mask_off = false;
  else
    g->feature_mask_off = true;

  if(!all.holdout_set_off)
  {
    all.sd->holdout_best_loss = FLT_MAX;
    if(vm.count("early_terminate"))      
      g->early_stop_thres = vm["early_terminate"].as< size_t>();     
  }
    
  sl_t sl = {g,save_load};
  learner ret(g,driver,learn,finish,sl);

  return ret;
}
}
