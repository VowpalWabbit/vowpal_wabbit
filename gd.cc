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

void* gd_thread(void *in)
{
  gd_thread_params* params = (gd_thread_params*) in;
  regressor reg = params->reg;

  example* ec = NULL;
  size_t current_pass = 0;

  
  while ( true )
    {//this is a poor man's select operation.
      if ((ec = get_example()) != NULL)//semiblocking operation.
	{
	  assert(ec->in_use);
	  if (ec->pass != current_pass)
	    {
	      if(global.span_server != "") {
		if(global.adaptive)
		  accumulate_weighted_avg(global.span_server, params->reg);
		else 
		  accumulate_avg(global.span_server, params->reg, 0);	      
	      }

	      if (global.save_per_pass)
		sync_weights(&reg);
	      global.eta *= global.eta_decay_rate;
	      save_predictor(*(params->final_regressor_name), current_pass);
	      current_pass = ec->pass;
	    }
	  
	  if (!command_example(ec, params))
	    {
	      predict(reg,ec,*(params->vars));
	      if (ec->eta_round != 0.)
		{
		  if (global.adaptive)
		    adaptive_inline_train(reg,ec,ec->eta_round);
		  else
		    inline_train(reg, ec, ec->eta_round);
		  if (global.sd->contraction < 1e-10)  // updating weights now to avoid numerical instability
		    sync_weights(&reg);
		  
		}
	    }
	  finish_example(ec);
	}
      else if (parser_done())
	{
	  sync_weights(&reg);
	  if(global.span_server != "") {
	    if(global.adaptive)
	      accumulate_weighted_avg(global.span_server, params->reg);
	    else 
	      accumulate_avg(global.span_server, params->reg, 0);	      
	  }
	  return NULL;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
  return NULL;
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

bool command_example(example* ec, gd_thread_params* params) {
  if (ec->indices.index() > 1)
    return false;

  if (ec->tag.index() >= 4 && !strncmp((const char*) ec->tag.begin, "save", 4))
    {
      string final_regressor_name = *(params->final_regressor_name);

      if ((ec->tag).index() >= 6 && (ec->tag)[4] == '_')
	final_regressor_name = string(ec->tag.begin+5, (ec->tag).index()-5);

      if (!global.quiet)
	cerr << "saving regressor to " << final_regressor_name << endl;
      dump_regressor(final_regressor_name, *(global.reg));

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
  output_and_account_example(ec);
  free_example(ec);
}

void print_update(example *ec)
{
  if (global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)
    {
      label_data* ld = (label_data*) ec->ld;
      char label_buf[32];
      if (ld->label == FLT_MAX)
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf,"%8.4f",ld->label);

      fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8.4f %8lu\n",
	      global.sd->sum_loss/global.sd->weighted_examples,
	      global.sd->sum_loss_since_last_dump / (global.sd->weighted_examples - global.sd->old_weighted_examples),
	      (long int)global.sd->example_number,
	      global.sd->weighted_examples,
	      label_buf,
	      ec->final_prediction,
	      (long unsigned int)ec->num_features);
     
      global.sd->sum_loss_since_last_dump = 0.0;
      global.sd->old_weighted_examples = global.sd->weighted_examples;
      global.sd->dump_interval *= 2;
    }
}

float query_decision(example*, float k);

void output_and_account_example(example* ec)
{
  label_data* ld = (label_data*)ec->ld;
  global.sd->weighted_examples += ld->weight;
  global.sd->weighted_labels += ld->label == FLT_MAX ? 0 : ld->label * ld->weight;
  global.sd->total_features += ec->num_features;
  global.sd->sum_loss += ec->loss;
  global.sd->sum_loss_since_last_dump += ec->loss;
  
  global.print(global.raw_prediction, ec->partial_prediction, -1, ec->tag);

  float ai=-1;
  if(global.active && ld->label == FLT_MAX)
    ai=query_decision(ec, global.sd->weighted_unlabeled_examples);
  global.sd->weighted_unlabeled_examples += ld->label == FLT_MAX ? ld->weight : 0;
  
  for (size_t i = 0; i<global.final_prediction_sink.index(); i++)
    {
      int f = global.final_prediction_sink[i];
      if(global.active)
	global.print(f, ec->final_prediction, ai, ec->tag);
      else if (global.lda > 0)
	print_lda_result(f,ec->topic_predictions.begin,0.,ec->tag);
      else
	global.print(f, ec->final_prediction, 0, ec->tag);
    }

  pthread_mutex_lock(&output_lock);
  global.local_example_number++;
  pthread_cond_signal(&output_done);
  pthread_mutex_unlock(&output_lock);

  global.sd->example_number++;

  print_update(ec);
}

float inline_predict_trunc(regressor &reg, example* &ec)
{
  float prediction = get_initial(ec->ld);
  
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
  float prediction = get_initial(ec->ld);

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

void print_audit_quad(weight* weights, audit_data& page_feature, v_array<audit_data> &offer_features, size_t mask, vector<string_value>& features)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;

  for (audit_data* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      ostringstream tempstream;
      tempstream << '\t' << page_feature.space << '^' << page_feature.feature << '^' 
		 << ele->space << '^' << ele->feature << ':' << (((halfhash + ele->weight_index)/global.stride) & mask)
		 << ':' << ele->x*page_feature.x
		 << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & mask], global.sd->gravity) * global.sd->contraction;
      string_value sv = {weights[ele->weight_index & mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

void print_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, vector<string_value>& features)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      ostringstream tempstream;
      cout << '\t' << (((halfhash + ele->weight_index)/global.stride) & mask) 
	   << ':' << (ele->x*page_feature.x)
	   << ':' << trunc_weight(weights[(halfhash + ele->weight_index) & mask], global.sd->gravity) * global.sd->contraction;
      string_value sv = {weights[ele->weight_index & mask]*ele->x, tempstream.str()};
      features.push_back(sv);
    }
}

void print_features(regressor &reg, example* &ec)
{
  weight* weights = reg.weight_vectors;
  size_t mask = global.weight_mask;
  size_t stride = global.stride;

  if (global.lda > 0)
    {
      size_t count = 0;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	count += ec->audit_features[*i].index() + ec->atomics[*i].index();
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	  {
	    cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index/global.stride << ':' << f->x;
	    for (size_t k = 0; k < global.lda; k++)
	      cout << ':' << weights[(f->weight_index+k) & mask];
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
	      tempstream << f->space << '^' << f->feature << ':' << f->weight_index/stride << ':' << f->x;
	      tempstream  << ':' << trunc_weight(weights[f->weight_index & mask], global.sd->gravity) * global.sd->contraction;
	      if(global.adaptive)
		tempstream << '@' << weights[(f->weight_index+1) & mask];
	      string_value sv = {weights[f->weight_index & mask]*f->x, tempstream.str()};
	      features.push_back(sv);
	    }
	else
	  for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	    {
	      ostringstream tempstream;
	      if ( f->weight_index == ((constant*stride)&global.weight_mask))
		tempstream << "Constant:";
	      tempstream << f->weight_index/stride << ':' << f->x;
	      tempstream << ':' << trunc_weight(weights[f->weight_index & mask], global.sd->gravity) * global.sd->contraction;
	      if(global.adaptive)
		tempstream << '@' << weights[(f->weight_index+1) & mask];
	      string_value sv = {weights[f->weight_index & mask]*f->x, tempstream.str()};
	      features.push_back(sv);
	    }
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
	if (ec->audit_features[(int)(*i)[0]].begin != ec->audit_features[(int)(*i)[0]].end)
	  for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f != ec->audit_features[(int)(*i)[0]].end; f++)
	    print_audit_quad(weights, *f, ec->audit_features[(int)(*i)[1]], global.weight_mask, features);
	else
	  for (feature* f = ec->atomics[(int)(*i)[0]].begin; f != ec->atomics[(int)(*i)[0]].end; f++)
	    print_quad(weights, *f, ec->atomics[(int)(*i)[1]], global.weight_mask, features);      

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

void one_pf_quad_adaptive_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, float g, example* ec, size_t& ctr)
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
  
  float g = reg.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  size_t ctr = 0;
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
	    one_pf_quad_adaptive_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, update, g, ec, ctr);
	} 
    }
}

float xGx_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float g, float& magx)
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
      magx += fabsf(ele->x);
    }
  return xGx;
}

float compute_xGx(regressor &reg, example* &ec, float& magx)
{//We must traverse the features in _precisely_ the same order as during training.
  size_t mask = global.weight_mask;
  label_data* ld = (label_data*)ec->ld;
  float g = reg.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;
  if (g==0) return 0.;

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
	  magx += fabsf(f->x);
	}
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    xGx += xGx_quad(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, g, magx);
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

float get_active_coin_bias(float k, float l, float g, float c0)
{
  float b,sb,rs,sl;
  b=c0*(log(k+1.)+0.0001)/(k+0.0001);
  sb=sqrt(b);
  if (l > 1.0) { l = 1.0; } else if (l < 0.0) { l = 0.0; } //loss should be in [0,1]
  sl=sqrt(l)+sqrt(l+g);
  if (g<=sb*sl+b)
    return 1;
  rs = (sl+sqrt(sl*sl+4*g))/(2*g);
  return b*rs*rs;
}

float query_decision(example* ec, float k)
{
  float bias, avg_loss, weighted_queries;
  if (k<=1.)
    bias=1.;
  else{
    weighted_queries = global.initial_t + global.sd->weighted_examples - global.sd->weighted_unlabeled_examples;
    avg_loss = global.sd->sum_loss/k + sqrt((1.+0.5*log(k))/(weighted_queries+0.0001));
    bias = get_active_coin_bias(k, avg_loss, ec->revert_weight/k, global.active_c0);
  }
  if(drand48()<bias)
    return 1./bias;
  else
    return -1.;
}

void local_predict(example* ec, gd_vars& vars, regressor& reg)
{
  label_data* ld = (label_data*)ec->ld;

  set_minmax(ld->label);

  ec->final_prediction = finalize_prediction(ec->partial_prediction * global.sd->contraction);

  if(global.active_simulation){
    float k = ec->example_t - ld->weight;
    ec->revert_weight = reg.loss->getRevertingWeight(ec->final_prediction, global.eta/pow(k,vars.power_t));
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
      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
      
      if (global.training)
	{
	  double eta_t;
	  float norm;
	  if (global.adaptive && global.exact_adaptive_norm) {
	    float magx = 0.;
	    norm = compute_xGx(reg, ec, magx);
	    eta_t = global.eta * norm / magx;
	  } else {
	    eta_t = global.eta / pow(t,vars.power_t) * ld->weight;
	    norm = global.nonormalize ? 1. : ec->total_sum_feat_sq;
	  }
	  
	  ec->eta_round = reg.loss->getUpdate(ec->final_prediction, ld->label, eta_t, norm) / global.sd->contraction;
	  
	  if (global.reg_mode && fabs(ec->eta_round) > 1e-8) {
	    double dev1 = reg.loss->first_derivative(ec->final_prediction, ld->label);
	    double eta_bar = (fabs(dev1) > 1e-8) ? (-ec->eta_round / dev1) : 0.0;
	    if (fabs(dev1) > 1e-8)
	      global.sd->contraction /= (1. + global.l2_lambda * eta_bar * norm);
	    global.sd->gravity += eta_bar * sqrt(norm) * global.l1_lambda;
	  }
	}
    }
  else if(global.active)
    ec->revert_weight = reg.loss->getRevertingWeight(ec->final_prediction, global.eta/pow(t,vars.power_t));

  if (global.audit)
    print_audit_features(reg, ec);
}

void predict(regressor& r, example* ex, gd_vars& vars)
{
  float prediction;
  if (global.reg_mode % 2)
    prediction = inline_predict_trunc(r, ex);
  else
    prediction = inline_predict(r, ex);

  ex->partial_prediction += prediction;

  local_predict(ex, vars,r);
  ex->done = true;
}

// trains regressor r on one example ex.
void train_one_example(regressor& r, example* ex, gd_vars& vars)
{
  predict(r,ex,vars);
  label_data* ld = (label_data*) ex->ld;
  if (ld->label != FLT_MAX && global.training) 
    inline_train(r, ex, ex->eta_round);
}

pthread_t* thread;
gd_thread_params* passers;

void setup_gd(gd_thread_params t)
{
  thread = (pthread_t*)calloc(1,sizeof(pthread_t));
  passers = (gd_thread_params*)calloc(1, sizeof(gd_thread_params));
  *passers = t;

  pthread_create(thread, NULL, gd_thread, (void *) passers);
}

void destroy_gd()
{
  pthread_join(*thread, NULL);
  free(passers);
  free(thread);
}

