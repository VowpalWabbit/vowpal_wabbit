/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <fstream>
#include <float.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "cache.h"
#include "simple_label.h"

using namespace std;

void mf_local_predict(example* ec, regressor& reg);

float mf_inline_predict(vw& all, example* &ec)
{
  float prediction = 0.0;

  weight* weights = all.reg.weight_vectors;
  size_t mask = all.weight_mask;

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
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0 && ec->atomics[(int)(*i)[1]].index() > 0)
	{
	  for (size_t k = 1; k <= all.rank; k++)
	    {
	      // x_l * l^k
	      // l^k is from index+1 to index+all.rank
	      float x_dot_l = sd_offset_add(weights, mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k);
	      // x_r * r^k
	      // r^k is from index+all.rank+1 to index+2*all.rank
	      float x_dot_r = sd_offset_add(weights, mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+all.rank);

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

void mf_inline_train(vw& all, example* &ec, float update)
{
      weight* weights = all.reg.weight_vectors;
      size_t mask = all.weight_mask;
      label_data* ld = (label_data*)ec->ld;

      // use final prediction to get update size
      // update = eta_t*(y-y_hat) where eta_t = eta/(3*t^p) * importance weight
      float eta_t = all.eta/pow(ec->example_t,all.power_t) / 3.f * ld->weight;
      update = all.loss->getUpdate(ec->final_prediction, ld->label, eta_t, 1.); //ec->total_sum_feat_sq);

      float regularization = eta_t * all.l2_lambda;

      // linear update
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	sd_offset_update(weights, mask, ec->atomics[*i].begin, ec->atomics[*i].end, 0, update, regularization);
      
      // quadratic update
      for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
	{
	  if (ec->atomics[(int)(*i)[0]].index() > 0 && ec->atomics[(int)(*i)[1]].index() > 0)
	    {

	      // update l^k weights
	      for (size_t k = 1; k <= all.rank; k++)
		{
		  // r^k \cdot x_r
		  float r_dot_x = ec->topic_predictions[2*k];
		  // l^k <- l^k + update * (r^k \cdot x_r) * x_l
		  sd_offset_update(weights, mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k, update*r_dot_x, regularization);
		}

	      // update r^k weights
	      for (size_t k = 1; k <= all.rank; k++)
		{
		  // l^k \cdot x_l
		  float l_dot_x = ec->topic_predictions[2*k-1];
		  // r^k <- r^k + update * (l^k \cdot x_l) * x_r
		  sd_offset_update(weights, mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+all.rank, update*l_dot_x, regularization);
		}

	    }
	}
}  

void mf_print_offset_features(vw& all, example* &ec, size_t offset)
{
  weight* weights = all.reg.weight_vectors;
  size_t mask = all.weight_mask;
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
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
    if (ec->atomics[(int)(*i)[0]].index() > 0 && ec->atomics[(int)(*i)[1]].index() > 0)
      {
	/* print out nsk^feature:hash:value:weight:nsk^feature^:hash:value:weight:prod_weights */
	for (size_t k = 1; k <= all.rank; k++)
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

void mf_print_audit_features(vw& all, example* ec, size_t offset)
{
  print_result(fileno(stdout),ec->final_prediction,-1,ec->tag);
  mf_print_offset_features(all, ec, offset);
}

void mf_local_predict(vw& all, example* ec)
{
  label_data* ld = (label_data*)ec->ld;
  all.set_minmax(all.sd, ld->label);

  ec->final_prediction = finalize_prediction(all, ec->partial_prediction);

  if (ld->label != FLT_MAX)
    {
      ec->loss = all.loss->getLoss(all.sd, ec->final_prediction, ld->label) * ld->weight;
    }

  if (all.audit)
    mf_print_audit_features(all, ec, 0);

}

float mf_predict(vw& all, example* ex)
{
  float prediction = mf_inline_predict(all, ex);

  ex->partial_prediction = prediction;
  mf_local_predict(all, ex);

  return ex->final_prediction;
}

void drive_gd_mf(void* in)
{
  vw* all = (vw*)in;
  example* ec = NULL;
  
  size_t current_pass = 0;
  while ( true )
    {
      if ((ec = get_example(all->p)) != NULL)//blocking operation.
	{
	  if (ec->pass != current_pass) {
	    all->eta *= all->eta_decay_rate;
	    save_predictor(*all, all->final_regressor_name, current_pass);
	    current_pass = ec->pass;
	  }
	  if (!command_example(*all, ec))
	    {
	      mf_predict(*all,ec);
	      if (all->training && ((label_data*)(ec->ld))->label != FLT_MAX)
		mf_inline_train(*all, ec, ec->eta_round);
	    }
	  finish_example(*all, ec);
	}
      else if (parser_done(all->p))
	return;
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
}
