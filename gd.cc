/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <fstream>
#include <float.h>
#include <netdb.h>
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
      else if ((ec = get_example(thread_num)) != NULL)//blocking operation.
	{
	  label_data* ld = (label_data*)ec->ld;
	  if ( ((ld->tag).begin != (ld->tag).end) 
	       && ((ld->tag)[0] == 's')&&((ld->tag)[1] == 'a')&&((ld->tag)[2] == 'v')&&((ld->tag)[3] == 'e'))
	    {
	      if ((*(params->final_regressor_name)) != "") 
		{
		  ofstream tempOut;
		  tempOut.open((*(params->final_regressor_name)).c_str());
		  dump_regressor(tempOut, reg);
		}
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

float finalize_prediction(float ret, size_t num_features, gd_vars& vars, float &norm) 
{
  if (num_features > 0)
    norm = 1. / sqrtf(num_features);
  else 
    norm = 1.;
  ret *= norm;
  if (isnan(ret))
    return 0.5;
  if ( ret > vars.max_prediction )
    return vars.max_prediction;
  if (ret < vars.min_prediction)
    return vars.min_prediction;
  return ret;

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
  
  if ( thread_num == 0 ) 
    prediction += weights[0];
  
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

  if ( thread_num == 0 )
    prediction += weights[offset & thread_mask];

  return prediction;
}

void print_offset_features(regressor &reg, example* &ec, size_t offset)
{
  weight* weights = reg.weight_vectors[0];
  size_t thread_mask = global.thread_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
      for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	{
	  // old
	  //	  cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index << ':' << f->x;
	  // new
	  cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index <<"(" << ((f->weight_index + offset) & thread_mask)  << ")" << ':' << f->x;

	  cout << ':' << weights[(f->weight_index + offset) & thread_mask];
	}
    else
      for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	{
	  cout << '\t' << f->weight_index << ':' << f->x;
	  cout << ':' << weights[(f->weight_index + offset) & thread_mask];
	}
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    if (ec->audit_features[(int)(*i)[0]].begin != ec->audit_features[(int)(*i)[0]].end)
      for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f != ec->audit_features[(int)(*i)[0]].end; f++)
	print_offset_audit_quad(weights, *f, ec->audit_features[(int)(*i)[1]], global.thread_mask, offset);
    else
      for (feature* f = ec->atomics[(int)(*i)[0]].begin; f != ec->atomics[(int)(*i)[0]].end; f++)
	print_offset_quad(weights, *f, ec->atomics[(int)(*i)[1]], global.thread_mask, offset);      

  cout << "\tConstant:0:1:" << weights[offset & global.thread_mask] << endl;
}

void print_audit_features(regressor &reg, example* ec, size_t offset)
{
  label_data* ld = (label_data*) ec->ld;
  print_result(fileno(stdout),ec->final_prediction,ld->tag);
  print_offset_features(reg, ec, offset);
}

void one_pf_quad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  update *= page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    weights[(halfhash + ele->weight_index) & mask] += update * ele->x;
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
  if (fabs(update) > 0.)
    {
      weight* weights = reg.weight_vectors[thread_num];
      size_t thread_mask = global.thread_mask;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	{
	  feature *f = ec->subsets[*i][thread_num];
	  for (; f != ec->subsets[*i][thread_num+1]; f++)
	    weights[f->weight_index & thread_mask] += update * f->x;
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
      
      if ( thread_num == 0 )
	weights[0] += update;
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
      
      if ( thread_num == 0 )
	weights[offset & thread_mask] += update;
    }
}  

void train(weight* weights, const v_array<feature> &features, float update)
{
  if (fabs(update) > 0.)
    for (feature* j = features.begin; j != features.end; j++)
      weights[j->weight_index] += update * j->x;
}

void local_predict(example* ec, size_t num_threads, gd_vars& vars, regressor& reg)
{
  label_data* ld = (label_data*)ec->ld;

  float norm;
  ec->final_prediction = 
    finalize_prediction(ec->partial_prediction, ec->num_features, vars, norm);

  if (global.local_prediction > 0)
    {
      prediction pred = {ec->final_prediction, ec->example_counter}; 
      send_prediction(global.local_prediction, pred);
      if (global.unique_id == 0)
	{
	  size_t len = sizeof(ld->label) + sizeof(ld->weight);
	  char c[len];
	  bufcache_simple_label(ld,c);
	  write(global.local_prediction,c,len);
	}
      fsync(global.local_prediction);
    }

  if (global.audit)
    print_audit_features(reg, ec, 0);

  if (ld->label != FLT_MAX)
    {
      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
      vars.t += ld->weight;

      ec->eta_round = vars.eta/pow(vars.t,vars.power_t)
	* (ld->label - ec->final_prediction)
	* norm * ld->weight;
      
      float example_update = reg.loss->getUpdate(ec->final_prediction, ld->label) * ld->weight;
      ec->eta_round = vars.eta/pow(vars.t,vars.power_t) * example_update * norm;
      if (ld->undo)
	ec->eta_round = -ec->eta_round;
    }
}

pthread_cond_t finished_sum = PTHREAD_COND_INITIALIZER;

float predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars)
{
  float prediction = inline_predict(r, ex, thread_num);

  pthread_mutex_lock(&ex->lock);

  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish != 0)
    {
      while (!ex->done)
	pthread_cond_wait(&finished_sum, &ex->lock);
    }
  else // We are the last thread using this example.
    {
      local_predict(ex, global.num_threads(),vars,r);
      ex->done = true;

      pthread_cond_broadcast(&finished_sum);

      if (global.training && ((label_data*)(ex->ld))->label != FLT_MAX)
	delay_example(ex,global.num_threads());
      else
	delay_example(ex,0);
    }
  pthread_mutex_unlock(&ex->lock);
  return ex->final_prediction;
}

float offset_predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars, size_t offset)
{
  float prediction = inline_offset_predict(r, ex, thread_num, offset);
  pthread_mutex_lock(&ex->lock);
  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish != 0) 
    pthread_cond_wait(&finished_sum, &ex->lock);
  else // We are the last thread using this example.
    {
      local_predict(ex, global.num_threads(),vars,r);
      pthread_cond_broadcast(&finished_sum);
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

