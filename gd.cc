/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <fstream>
#include <float.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"

void* gd_thread(void *in)
{
  gd_thread_params* params = (gd_thread_params*) in;
  regressor reg = params->reg;
  size_t thread_num = params->thread_num;
  example* ec = NULL;

  while ( true )
    {//this is a poor man's select operation.
      if ((ec = get_delay_example(thread_num)) != NULL)//nonblocking
	{
	  inline_train(reg, ec, thread_num, ec->eta_round);
	  finish_example(ec);
	}
      else if ((ec = get_example(thread_num)) != NULL)//semiblocking operation.
	{
	  assert(ec->in_use);
	  if ( (ec->tag).end == (ec->tag).begin+4 
	       && ((ec->tag)[0] == 's')&&((ec->tag)[1] == 'a')&&((ec->tag)[2] == 'v')&&((ec->tag)[3] == 'e'))
	    {
	      if ((*(params->final_regressor_name)) != "") 
		dump_regressor(*(params->final_regressor_name), reg);
	      delay_example(ec,0);
	    }
	  else
	    predict(reg,ec,thread_num,*(params->vars));
	}
      else if (thread_done(thread_num))
	{
	  if (global.local_prediction > 0)
	    shutdown(global.local_prediction, SHUT_WR);
	  return NULL;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }

  return NULL;
}

float finalize_prediction(float ret) 
{
  if (isnan(ret))
    return 0.5;
  if ( ret > global.max_label )
    return global.max_label;
  if (ret < global.min_label)
    return global.min_label;
  return ret;
}

void finish_example(example* ec)
{
  pthread_mutex_lock(&ec->lock);
  if (-- ec->threads_to_finish == 0)
    {
      pthread_mutex_unlock(&ec->lock);

      output_and_account_example(ec);
      free_example(ec);
    }
  else
    pthread_mutex_unlock(&ec->lock);
}

void print_update(example *ec)
{
  if (global.weighted_examples > global.dump_interval && !global.quiet)
    {
      label_data* ld = (label_data*) ec->ld;
      char label_buf[32];
      if (ld->label == FLT_MAX)
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf,"%8.4f",ld->label);

      fprintf(stderr, "%-10.6f %-10.6f %8lld %8.1f   %s %8.4f %8lu\n",
	      global.sum_loss/global.weighted_examples,
	      global.sum_loss_since_last_dump / (global.weighted_examples - global.old_weighted_examples),
	      global.example_number,
	      global.weighted_examples,
	      label_buf,
	      ec->final_prediction,
	      (long unsigned int)ec->num_features);
     
      global.sum_loss_since_last_dump = 0.0;
      global.old_weighted_examples = global.weighted_examples;
      global.dump_interval *= 2;
    }
}

float query_decision(example*, float k);

void output_and_account_example(example* ec)
{
  global.example_number++;
  label_data* ld = (label_data*)ec->ld;
  global.weighted_examples += ld->weight;
  global.weighted_labels += ld->label == FLT_MAX ? 0 : ld->label * ld->weight;
  global.total_features += ec->num_features;
  global.sum_loss += ec->loss;
  global.sum_loss_since_last_dump += ec->loss;
  
  global.print(global.raw_prediction, ec->partial_prediction, -1, ec->tag);

  float ai=-1;
  if(global.active && ld->label == FLT_MAX)
    ai=query_decision(ec, global.weighted_unlabeled_examples);
  global.weighted_unlabeled_examples += ld->label == FLT_MAX ? ld->weight : 0;
  
  for (size_t i = 0; i<global.final_prediction_sink.index(); i++)
    {
      int f = global.final_prediction_sink[i].fd;
      if(global.active)
	global.print(f, ec->final_prediction, ai, ec->tag);
      else if (global.lda > 0)
	print_lda_result(f,ec->topic_predictions.begin,0.,ec->tag);
      else
	{
	  float w;
	  if (global.reg->weight_vectors != NULL) {
	    w = global.reg->weight_vectors[0][global.final_prediction_sink[i].id];
	  } else {
	    w = 0.;
	  }
	  global.print(f, ec->final_prediction, w*ec->global_weight, ec->tag);
	}
    }
  print_update(ec);
}

float inline_predict(regressor &reg, example* &ec, size_t thread_num)
{
  float prediction = 0.0;

  weight* weights = reg.weight_vectors[thread_num];
  size_t thread_mask = global.thread_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    prediction += sd_add(weights,thread_mask,ec->subsets[*i][thread_num], ec->subsets[*i][thread_num+1]);
  
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->subsets[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	  temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	  for (; temp.begin != temp.end; temp.begin++)
	    prediction += one_pf_quad_predict(weights,*temp.begin,
					      ec->atomics[(int)(*i)[1]],thread_mask);
	}
    }
  
  return prediction;
}

float inline_offset_predict(regressor &reg, example* &ec, size_t thread_num, size_t offset)
{
  float prediction = 0.0;

  weight* weights = reg.weight_vectors[thread_num];
  size_t thread_mask = global.thread_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    for (feature *f = ec->subsets[*i][thread_num]; f != ec->subsets[*i][thread_num+1]; f++)
      prediction += weights[(f->weight_index + offset) & thread_mask] * f->x;
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->subsets[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	  temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	  for (; temp.begin != temp.end; temp.begin++)
	    prediction += offset_quad_predict(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, offset);
	}
    }

  return prediction;
}

void print_features(regressor &reg, example* &ec)
{
  weight* weights = reg.weight_vectors[0];
  size_t thread_mask = global.thread_mask;
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
	      cout << ':' << weights[(f->weight_index+k) & thread_mask];
	  }
      cout << " total of " << count << " features." << endl;
    }
  else
    {
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
	  for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	    {
	      cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index/stride << ':' << f->x;
	      
	      cout << ':' << weights[f->weight_index & thread_mask];
	      if(global.adaptive)
		cout << '@' << weights[(f->weight_index+1) & thread_mask];
	    }
	else
	  for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	    {
	      cout << '\t';
	      if ( f->weight_index == (constant&global.mask)*stride)
		cout << "Constant:";
	      cout << f->weight_index/stride << ':' << f->x;
	      cout << ':' << weights[f->weight_index & thread_mask];
	      if(global.adaptive)
		cout << '@' << weights[(f->weight_index+1) & thread_mask];
	    }
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
	if (ec->audit_features[(int)(*i)[0]].begin != ec->audit_features[(int)(*i)[0]].end)
	  for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f != ec->audit_features[(int)(*i)[0]].end; f++)
	    print_audit_quad(weights, *f, ec->audit_features[(int)(*i)[1]], global.thread_mask);
	else
	  for (feature* f = ec->atomics[(int)(*i)[0]].begin; f != ec->atomics[(int)(*i)[0]].end; f++)
	    print_quad(weights, *f, ec->atomics[(int)(*i)[1]], global.thread_mask);      
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

void one_pf_quad_adaptive_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, float g)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float update2 = g * page_feature.x * page_feature.x;
  update *= page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[1] += update2 * ele->x * ele->x;
      w[0] += update * ele->x*InvSqrt(w[1]);
    }
}

void offset_quad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, size_t offset)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index + offset;
  update *= page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    weights[(halfhash + ele->weight_index) & mask] += update * ele->x;
}

void inline_train(regressor &reg, example* &ec, size_t thread_num, float update)
{
  if (fabs(update)>0.)
  {
    size_t thread_mask = global.thread_mask;
    if (global.adaptive)
    {
      label_data* ld = (label_data*)ec->ld;
      float g = reg.loss->getSquareGrad(ec->final_prediction, ld->label) * ld->weight;

      //assert((g>0 && fabs(update)>0) || (g==0 && update==0));
      weight* weights = reg.weight_vectors[thread_num];
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
      {
	feature *f = ec->subsets[*i][thread_num];
	for (; f != ec->subsets[*i][thread_num+1]; f++)
	{
	  weight* w = &weights[f->weight_index & thread_mask];
	  w[1] += g * f->x * f->x;
	  w[0] += update * f->x*InvSqrt(w[1]);
	}
      }
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
      {
	if (ec->subsets[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	  temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	  for (; temp.begin != temp.end; temp.begin++)
	    one_pf_quad_adaptive_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, update, g);
	} 
      }
    } else {
      weight* weights = reg.weight_vectors[thread_num];
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	{
	  feature *f = ec->subsets[*i][thread_num];
	  for (; f != ec->subsets[*i][thread_num+1]; f++){
	    weights[f->weight_index & thread_mask] += update * f->x;
	  }
	}
      
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
	{
	  if (ec->subsets[(int)(*i)[0]].index() > 0)
	    {
	      v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	      temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	      temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	      for (; temp.begin != temp.end; temp.begin++)
		one_pf_quad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, update);
	    } 
	}
    }
  }
}  

void offset_train(regressor &reg, example* &ec, size_t thread_num, float update, size_t offset)
{
  if (fabs(update) > 0.)
    {
      weight* weights = reg.weight_vectors[thread_num];
      size_t thread_mask = global.thread_mask;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	for (feature *f = ec->subsets[*i][thread_num]; f != ec->subsets[*i][thread_num+1]; f++)
	  weights[(f->weight_index+offset) & thread_mask] += update * f->x;
      
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
	{
	  if (ec->subsets[(int)(*i)[0]].index() > 0)
	    {
	      v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	      temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	      temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	      for (; temp.begin != temp.end; temp.begin++)
		offset_quad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, update, offset);
	    } 
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
    weighted_queries = global.initial_t + global.weighted_examples - global.weighted_unlabeled_examples;
    avg_loss = global.sum_loss/k + sqrt((1.+0.5*log(k))/(weighted_queries+0.0001));
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

  ec->final_prediction = 
    finalize_prediction(ec->partial_prediction);

  if(global.active_simulation){
    float k = ec->example_t - ld->weight;
    ec->revert_weight = reg.loss->getRevertingWeight(ec->final_prediction, global.eta/pow(k,vars.power_t));
    float importance = query_decision(ec, k);
    if(importance > 0){
      global.queries += 1;
      ld->weight *= importance;
    }
    else //do not query => do not train
      ld->label = FLT_MAX;
  }

  float t;
  if(global.active)
    t = global.weighted_unlabeled_examples;
  else
    t = ec->example_t;

  if (ld->label != FLT_MAX)
    {
      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
      //Using the euclidean norm is faster but probably not as good as the adaptive norm defined by the learning rates
      ec->eta_round = reg.loss->getUpdate(ec->final_prediction, ld->label, global.eta/pow(t,vars.power_t)*ld->weight, ec->total_sum_feat_sq);
    }
  else if(global.active)
    ec->revert_weight = reg.loss->getRevertingWeight(ec->final_prediction, global.eta/pow(t,vars.power_t));

  if (global.delayed_global && global.local_prediction > 0)
    ec->eta_round = 0;

  if (global.local_prediction > 0)
    {
      prediction pred;
      pred.p = ec->final_prediction;
      if (global.training && ld->label != FLT_MAX  && global.backprop)
        pred.p += ec->eta_round * ec->total_sum_feat_sq;
      pred.example_number = ec->example_counter;
      send_prediction(global.local_prediction, pred);
      if (global.unique_id == 0)
	{
	  size_t len = sizeof(ld->label) + sizeof(ld->weight);
	  char c[len];
	  bufcache_simple_label(ld,c);
	  if (write(global.local_prediction,c,len) < (int)len)
	    cerr << "uhoh" << endl;
	}
    }

  if (global.audit)
    print_audit_features(reg, ec);
}

void predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars)
{
  float prediction = inline_predict(r, ex, thread_num);

  pthread_mutex_lock(&ex->lock);

  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish == 0)
    {//We are the last thread using this example
      local_predict(ex, vars,r);
      ex->done = true;
      
      pthread_cond_broadcast(&ex->finished_sum);
      
      if (global.training && ((label_data*)(ex->ld))->label != FLT_MAX)
	delay_example(ex,global.num_threads());
      else
	delay_example(ex,0);
    }
  else if (global.training && ((label_data*)(ex->ld))->label != FLT_MAX)
    //need to wait if there is training to do to keep example processing sequential
    {
      while (!ex->done)
	pthread_cond_wait(&ex->finished_sum, &ex->lock);
    }
  pthread_mutex_unlock(&ex->lock);
}

float offset_predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars, size_t offset)
{
  float prediction = inline_offset_predict(r, ex, thread_num, offset);
  pthread_mutex_lock(&ex->lock);
  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish != 0) 
    pthread_cond_wait(&ex->finished_sum, &ex->lock);
  else // We are the last thread using this example.
    {
      local_predict(ex, vars,r);
      pthread_cond_broadcast(&ex->finished_sum);
    }
  pthread_mutex_unlock(&ex->lock);
  return ex->final_prediction;
}

// trains regressor r on one example ex.
void train_one_example(regressor& r, example* ex, size_t thread_num, gd_vars& vars)
{
  predict(r,ex,thread_num,vars);
  label_data* ld = (label_data*) ex->ld;
  if (ld->label != FLT_MAX && global.training) 
    inline_train(r, ex, thread_num, ex->eta_round);
}

// trains regressor r on one example ex.
void train_offset_example(regressor& r, example* ex, size_t thread_num, gd_vars& vars, size_t offset)
{
  offset_predict(r,ex,thread_num,vars,offset);
  label_data* ld = (label_data*) ex->ld;
  if (ld->label != FLT_MAX && global.training) 
    offset_train(r, ex, thread_num, ex->eta_round, offset);
}

pthread_t* threads;
gd_thread_params** passers;
size_t num_threads;

void setup_gd(gd_thread_params t)
{
  num_threads = t.thread_num;
  threads = (pthread_t*)calloc(num_threads,sizeof(pthread_t));
  passers = (gd_thread_params**)calloc(num_threads,sizeof(gd_thread_params*));

  for (size_t i = 0; i < num_threads; i++)
    {
      passers[i] = (gd_thread_params*)calloc(1, sizeof(gd_thread_params));
      *(passers[i]) = t;
      passers[i]->thread_num = i;
      pthread_create(&threads[i], NULL, gd_thread, (void *) passers[i]);
    }
}

void destroy_gd()
{
  for (size_t i = 0; i < num_threads; i++) 
    {
      pthread_join(threads[i], NULL);
      free(passers[i]);
    }
  free(threads);
  free(passers);
}

