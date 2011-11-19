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
#include "simple_label.h"
#include "delay_ring.h"

using namespace std;

void mf_inline_train(gd_vars& vars, regressor &reg, example* &ec, float update);
void mf_local_predict(example* ec, gd_vars& vars, regressor& reg);
float mf_predict(regressor& r, example* ex, gd_vars& vars);

void* gd_mf_thread(void *in)
{
  gd_thread_params* params = (gd_thread_params*) in;
  regressor reg = params->reg;
  example* ec = NULL;

  size_t current_pass = 0;
  while ( true )
    {//this is a poor man's select operation.
      if ((ec = get_delay_example()) != NULL)//nonblocking
	{

	  if (ec->pass != current_pass) {
	    global.eta *= global.eta_decay_rate;
	    current_pass = ec->pass;
	  }

	  //cout << ec->eta_round << endl;
	  mf_inline_train(*(params->vars), reg, ec, ec->eta_round);
	  finish_example(ec);
	}
      else if ((ec = get_example()) != NULL)//blocking operation.
	{
	  if (command_example(ec, params))
	    {
	      delay_example(ec,0);
	    }
	  else
	    mf_predict(reg,ec,*(params->vars));
	}
      else if (thread_done())
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

float mf_inline_predict(regressor &reg, example* &ec)
{
  float prediction = 0.0;

  weight* weights = reg.weight_vectors;
  size_t mask = global.mask;

  // clear stored predictions
  ec->topic_predictions.erase();

  float linear_prediction = 0;
  // linear terms
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    linear_prediction += sd_add(weights,mask,ec->atomics[*i].begin, ec->atomics[*i].end);

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
	      float x_dot_l = sd_offset_add(weights, mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k);
	      // x_r * r^k
	      // r^k is from index+global.rank+1 to index+2*global.rank
	      float x_dot_r = sd_offset_add(weights, mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+global.rank);

	      prediction += x_dot_l * x_dot_r;

	      // store prediction from interaction terms
	      push(ec->topic_predictions, x_dot_l);
	      push(ec->topic_predictions, x_dot_r);
	    }
	}
    }
    
  // ec->topic_predictions has linear, x_dot_l_1, x_dot_r_1, x_dot_l_2, x_dot_r_2, ... 

  return prediction;
}

void mf_inline_train(gd_vars& vars, regressor &reg, example* &ec, float update)
{
      weight* weights = reg.weight_vectors;
      size_t mask = global.mask;
      label_data* ld = (label_data*)ec->ld;

      // use final prediction to get update size
      // update = eta_t*(y-y_hat) where eta_t = eta/(3*t^p) * importance weight
      float eta_t = global.eta/pow(ec->example_t,vars.power_t) / 3. * ld->weight;
      update = reg.loss->getUpdate(ec->final_prediction, ld->label, eta_t, 1.); //ec->total_sum_feat_sq);

      float regularization = eta_t * global.l2_lambda;

      // linear update
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	sd_offset_update(weights, mask, ec->atomics[*i].begin, ec->atomics[*i].end, 0, update, regularization);
      
      // quadratic update
      for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
	{
	  if (ec->atomics[(int)(*i)[0]].index() > 0 && ec->atomics[(int)(*i)[1]].index() > 0)
	    {

	      // update l^k weights
	      for (size_t k = 1; k <= global.rank; k++)
		{
		  // r^k \cdot x_r
		  float r_dot_x = ec->topic_predictions[2*k];
		  // l^k <- l^k + update * (r^k \cdot x_r) * x_l
		  sd_offset_update(weights, mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k, update*r_dot_x, regularization);
		}

	      // update r^k weights
	      for (size_t k = 1; k <= global.rank; k++)
		{
		  // l^k \cdot x_l
		  float l_dot_x = ec->topic_predictions[2*k-1];
		  // r^k <- r^k + update * (l^k \cdot x_l) * x_r
		  sd_offset_update(weights, mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+global.rank, update*l_dot_x, regularization);
		}

	    }
	}
}  

void mf_print_offset_features(regressor &reg, example* &ec, size_t offset)
{
  weight* weights = reg.weight_vectors;
  size_t mask = global.mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
      for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	{
	  cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index <<"(" << ((f->weight_index + offset) & mask)  << ")" << ':' << f->x;

	  cout << ':' << weights[(f->weight_index + offset) & mask];
	}
    else
      for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	{
	  cout << '\t' << f->weight_index << ':' << f->x;
	  cout << ':' << weights[(f->weight_index + offset) & mask];
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
		  cout << '\t' << f->space << k << '^' << f->feature << ':' << ((f->weight_index+k)&mask) 
		       <<"(" << ((f->weight_index + offset +k) & mask)  << ")" << ':' << f->x;
		  cout << ':' << weights[(f->weight_index + offset + k) & mask];

		  cout << ':' << f2->space << k << '^' << f2->feature << ':' << ((f2->weight_index+k)&mask) 
		       <<"(" << ((f2->weight_index + offset +k) & mask)  << ")" << ':' << f2->x;
		  cout << ':' << weights[(f2->weight_index + offset + k) & mask];

		  cout << ':' <<  weights[(f->weight_index + offset + k) & mask] * weights[(f2->weight_index + offset + k) & mask];

		}
	  }
      }
}

void mf_print_audit_features(regressor &reg, example* ec, size_t offset)
{
  print_result(fileno(stdout),ec->final_prediction,-1,ec->tag);
  mf_print_offset_features(reg, ec, offset);
}

void mf_local_predict(example* ec, gd_vars& vars, regressor& reg)
{
  label_data* ld = (label_data*)ec->ld;
  set_minmax(ld->label);

  ec->final_prediction = finalize_prediction(ec->partial_prediction);

  if (ld->label != FLT_MAX)
    {
      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
    }

  if (global.audit)
    mf_print_audit_features(reg, ec, 0);

}

float mf_predict(regressor& r, example* ex, gd_vars& vars)
{
  float prediction = mf_inline_predict(r, ex);

  ex->partial_prediction = prediction;
  mf_local_predict(ex, vars,r);

  if (global.training && ((label_data*)(ex->ld))->label != FLT_MAX)
    delay_example(ex,1);
  else
    delay_example(ex,0);
  
  return ex->final_prediction;
}

pthread_t* mf_threads;
gd_thread_params** mf_passers;
size_t mf_num_mf_threads;

void setup_gd_mf(gd_thread_params t)
{
  mf_threads = (pthread_t*)calloc(mf_num_mf_threads,sizeof(pthread_t));
  mf_passers = (gd_thread_params**)calloc(mf_num_mf_threads,sizeof(gd_thread_params*));

  for (size_t i = 0; i < mf_num_mf_threads; i++)
    {
      mf_passers[i] = (gd_thread_params*)calloc(1, sizeof(gd_thread_params));
      *(mf_passers[i]) = t;
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

