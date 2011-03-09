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
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"

void mf_inline_train(gd_vars& vars, regressor &reg, example* &ec, size_t thread_num, float update);
void mf_local_predict(example* ec, size_t num_threads, gd_vars& vars, regressor& reg);
float mf_predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars);

void* gd_mf_thread(void *in)
{
  gd_thread_params* params = (gd_thread_params*) in;
  regressor reg = params->reg;
  size_t thread_num = params->thread_num;
  example* ec = NULL;

  size_t current_pass = 0;
  while ( true )
    {//this is a poor man's select operation.
      if ((ec = get_delay_example(thread_num)) != NULL)//nonblocking
	{

	  if (ec->pass != current_pass) {
	    global.eta *= global.eta_decay_rate;
	    current_pass = ec->pass;
	  }

	  //cout << ec->eta_round << endl;
	  mf_inline_train(*(params->vars), reg, ec, thread_num, ec->eta_round);
	  finish_example(ec);
	}
      else if ((ec = get_example(thread_num)) != NULL)//blocking operation.
	{
	  if ( ((ec->tag).begin != (ec->tag).end) 
	       && ((ec->tag)[0] == 's')&&((ec->tag)[1] == 'a')&&((ec->tag)[2] == 'v')&&((ec->tag)[3] == 'e'))
	    {
	      if ((*(params->final_regressor_name)) != "") 
		{
		  dump_regressor(*(params->final_regressor_name), reg);
		}
	    }
	  else
	    mf_predict(reg,ec,thread_num,*(params->vars));
	}
      else if (thread_done(thread_num))
	{

	  // decay all weights by (1-lambda)^t if global.weight_decay is set
	  if (global.weight_decay > 0)
	    for (size_t i = 0; i < global.length(); i++) {
	      weight* weights_for_w = & (reg.weight_vectors[thread_num][i*global.stride]);
	      
	      for (size_t k = 0; k <= 2*global.rank; k++)
		weights_for_w[k] *= pow(1 - global.weight_decay, global.weighted_examples);
	    }

	  if (global.local_prediction > 0)
	    shutdown(global.local_prediction, SHUT_WR);
	  return NULL;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }

  return NULL;
}

float mf_inline_predict(regressor &reg, example* &ec, size_t thread_num)
{
  float prediction = 0.0;

  weight* weights = reg.weight_vectors[thread_num];
  size_t thread_mask = global.thread_mask;

  // clear stored predictions
  ec->topic_predictions.erase();

  float linear_prediction = 0;
  // linear terms
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    linear_prediction += sd_add(weights,thread_mask,ec->atomics[*i].begin, ec->atomics[*i].end);

  // yhat <- (1-lambda)^t * yhat
  if (global.weight_decay > 0)
    linear_prediction *= pow(1-global.weight_decay, ec->example_t);

  // store constant + linear prediction
  // note: constant is now automatically added
  push(ec->topic_predictions, linear_prediction);
  
  prediction += linear_prediction;

  // interaction terms
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0 && ec->atomics[(int)(*i)[1]].index() > 0)
	{
	  for (size_t k = 1; k <= global.rank; k++)
	    {
	      // x_l * l^k
	      // l^k is from index+1 to index+global.rank
	      float first = sd_offset_add(weights, thread_mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k);
	      // x_r * r^k
	      // r^k is from index+global.rank+1 to index+2*global.rank
	      float second = sd_offset_add(weights, thread_mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+global.rank);

	      // yhat <- (1-lambda)^t * yhat
	      if (global.weight_decay > 0) {
		first *= pow(1-global.weight_decay, ec->example_t);
		second *= pow(1-global.weight_decay, ec->example_t);
	      }

	      prediction += first * second;

	      // store prediction from interaction terms
	      push(ec->topic_predictions, first);
	      push(ec->topic_predictions, second);
	    }
	}
    }
    
  return prediction;
}

void mf_inline_train(gd_vars& vars, regressor &reg, example* &ec, size_t thread_num, float update)
{
      //cout << "update = " << update << endl;
      weight* weights = reg.weight_vectors[thread_num];
      size_t thread_mask = global.thread_mask;
      label_data* ld = (label_data*)ec->ld;

      // use topic_predictions for constant + linear update
      update = reg.loss->getUpdate(ec->topic_predictions[0], ld->label, global.eta/pow(ec->example_t,vars.power_t) / 3 * ld->weight, ec->total_sum_feat_sq);

      cout << "ec->topic_predictions[0]: " << ec->topic_predictions[0] << endl;
      cout << "ld->label: " << ld->label << endl;
      cout << "global.eta: " << global.eta << endl;
      cout << "power_t: " << vars.power_t << endl;
      cout << "ld->weight: " << ld->weight << endl;
      cout << "eta: " <<  global.eta/pow(ec->example_t,vars.power_t) / 3 * ld->weight << endl;
      cout << "ec->total_sum_feat_sq: " << ec->total_sum_feat_sq << endl;
      cout << "linear update: " << update << endl;

      // linear update
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	sd_offset_update(weights, thread_mask, ec->atomics[*i].begin, ec->atomics[*i].end, 0, update);
      
      // y - (old yhat linear + delta yhat linear)
      float residual_label = ld->label - ( ec->topic_predictions[0] + update * (ec->total_sum_feat_sq) );

      //cout << update * (ec->total_sum_feat_sq) << endl;

      // quadratic update
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
	{
	  if (ec->atomics[(int)(*i)[0]].index() > 0 && ec->atomics[(int)(*i)[1]].index() > 0)
	    {

	      /* all "left" updates */

	      float sum_rk_xr_sq = 0;
	      // compute \sum_k |r^k \cdot x_r|^2
	      // jntj: store r^k \cdot x_r's at some point?
	      float sum_topic_predictions = 0;
	      for (size_t k = 1; k <= global.rank; k++)
		{
		  float tmp = ec->topic_predictions[2*k];
		  sum_rk_xr_sq += tmp*tmp;
		  sum_topic_predictions += tmp*ec->topic_predictions[2*k-1];
		}

	      // |x_l|^2 \sum_k |r^k \cdot x_r|^2
	      float norm = ec->sum_feat_sq[(int)(*i)[0]] * sum_rk_xr_sq;
	      // \eta (y-\hat{y}) * norm
	      update = reg.loss->getUpdate(sum_topic_predictions, residual_label, global.eta/pow(ec->example_t,vars.power_t) / 3 * ld->weight, norm);

	      cout << "left update: " << update << endl;


	      // update l^k weights
	      for (size_t k = 1; k <= global.rank; k++)
		{
		  // (r^k \cdot x_r)
		  float r_dot_x = ec->topic_predictions[2*k];
		  // update l^k with above
		  sd_offset_update(weights, thread_mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k, update*r_dot_x);
		}

	      //cout << update*norm << endl;

	      /* all "right" updates */

	      float sum_lk_xl_sq = 0;
	      sum_topic_predictions += update * norm;
	      // compute \sum_k |l^k \cdot x_l|^2
	      for (size_t k = 1; k <= global.rank; k++)
		{
		  float tmp = ec->topic_predictions[2*k-1];
		  sum_lk_xl_sq += tmp*tmp;
		}

	      // |x_r|^2 \sum_k |l^k \cdot x_l|^2
	      norm = ec->sum_feat_sq[(int)(*i)[1]] * sum_lk_xl_sq;
	      // \eta (y-\hat{y}) * norm
	      update = reg.loss->getUpdate(sum_topic_predictions, residual_label, global.eta/pow(ec->example_t,vars.power_t) / 3 * ld->weight, norm);

	      cout << "right update: " << update << endl;

	      // update l^k weights
	      for (size_t k = 1; k <= global.rank; k++)
		{
		  // (l^k \cdot x_l)
		  float l_dot_x = ec->topic_predictions[2*k-1];
		  // update r^k with above
		  sd_offset_update(weights, thread_mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k, update*l_dot_x);
		}

	      //cout << update*norm << endl;

	    }
	}

}  

void mf_print_offset_features(regressor &reg, example* &ec, size_t offset)
{
  weight* weights = reg.weight_vectors[0];
  size_t thread_mask = global.thread_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
      for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	{
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
    if (ec->atomics[(int)(*i)[0]].index() > 0 && ec->atomics[(int)(*i)[1]].index() > 0)
      {
	/* print out nsk^feature:hash:value:weight:nsk^feature^:hash:value:weight:prod_weights */
	for (size_t k = 1; k <= global.rank; k++)
	  {
	    for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f!= ec->audit_features[(int)(*i)[0]].end; f++)
	      for (audit_data* f2 = ec->audit_features[(int)(*i)[1]].begin; f2!= ec->audit_features[(int)(*i)[1]].end; f2++)
		{
		  cout << '\t' << f->space << k << '^' << f->feature << ':' << ((f->weight_index+k)&thread_mask) 
		       <<"(" << ((f->weight_index + offset +k) & thread_mask)  << ")" << ':' << f->x;
		  cout << ':' << weights[(f->weight_index + offset + k) & thread_mask];

		  cout << ':' << f2->space << k << '^' << f2->feature << ':' << ((f2->weight_index+k)&thread_mask) 
		       <<"(" << ((f2->weight_index + offset +k) & thread_mask)  << ")" << ':' << f2->x;
		  cout << ':' << weights[(f2->weight_index + offset + k) & thread_mask];

		  cout << ':' <<  weights[(f->weight_index + offset + k) & thread_mask] * weights[(f2->weight_index + offset + k) & thread_mask];

		}
	  }
      }

  cout << "\tConstant:0:1:" << weights[(constant+offset) & global.thread_mask] << endl;

}

void mf_print_audit_features(regressor &reg, example* ec, size_t offset)
{
  print_result(fileno(stdout),ec->final_prediction,-1,ec->tag);
  mf_print_offset_features(reg, ec, offset);
}

void mf_local_predict(example* ec, size_t mf_num_threads, gd_vars& vars, regressor& reg)
{
  label_data* ld = (label_data*)ec->ld;

  ec->final_prediction = 
    finalize_prediction(ec->partial_prediction);

  if (ec->final_prediction > 5) {
    cout << "partial prediction " << ec->partial_prediction;
    cout << " , final prediction " << ec->final_prediction << endl;
  }

  if (ld->label != FLT_MAX)
    {
      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
    }

  // jntj: needs to be updated for cluster setting
  if (global.local_prediction > 0)
    {
      prediction pred = {ec->example_counter, ec->final_prediction}; 
      send_prediction(global.local_prediction, pred);
      if (global.unique_id == 0)
	{
	  size_t len = sizeof(ld->label) + sizeof(ld->weight);
	  char c[len];
	  bufcache_simple_label(ld,c);
	  write(global.local_prediction,c,len);
	}
    }

  if (global.audit)
    mf_print_audit_features(reg, ec, 0);

}

pthread_cond_t mf_finished_sum = PTHREAD_COND_INITIALIZER;

float mf_predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars)
{
  float prediction = mf_inline_predict(r, ex, thread_num);

  pthread_mutex_lock(&ex->lock);

  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish != 0)
    {
      while (!ex->done)
	pthread_cond_wait(&mf_finished_sum, &ex->lock);
    }
  else // We are the last thread using this example.
    {
      mf_local_predict(ex, global.num_threads(),vars,r);
      ex->done = true;

      pthread_cond_broadcast(&mf_finished_sum);

      if (global.training && ((label_data*)(ex->ld))->label != FLT_MAX)
	delay_example(ex,global.num_threads());
      else
	delay_example(ex,0);
    }
  pthread_mutex_unlock(&ex->lock);
  return ex->final_prediction;
}

pthread_t* mf_threads;
gd_thread_params** mf_passers;
size_t mf_num_mf_threads;

void setup_gd_mf(gd_thread_params t)
{
  mf_num_mf_threads = t.thread_num;
  mf_threads = (pthread_t*)calloc(mf_num_mf_threads,sizeof(pthread_t));
  mf_passers = (gd_thread_params**)calloc(mf_num_mf_threads,sizeof(gd_thread_params*));

  for (size_t i = 0; i < mf_num_mf_threads; i++)
    {
      mf_passers[i] = (gd_thread_params*)calloc(1, sizeof(gd_thread_params));
      *(mf_passers[i]) = t;
      mf_passers[i]->thread_num = i;
      pthread_create(&mf_threads[i], NULL, gd_mf_thread, (void *) mf_passers[i]);
    }
}

void destroy_gd_mf()
{
  for (size_t i = 0; i < mf_num_mf_threads; i++) 
    {
      pthread_join(mf_threads[i], NULL);
      free(mf_passers[i]);
    }
  free(mf_threads);
  free(mf_passers);
}

