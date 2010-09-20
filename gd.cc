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
	  //	  cout << "training with " << ec->eta_round << endl;
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
		{
		  ofstream tempOut;
		  tempOut.open((*(params->final_regressor_name)).c_str());
		  dump_regressor(tempOut, reg);
		}
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
      fprintf(stderr, "%-10.6f %-10.6f %8lld %8.1f   %8.4f %8.4f %8lu\n",
	      global.sum_loss/global.weighted_examples,
	      global.sum_loss_since_last_dump / (global.weighted_examples - global.old_weighted_examples),
	      global.example_number,
	      global.weighted_examples,
	      ld->label,
	      ec->final_prediction,
	      (long unsigned int)ec->num_features);
      
      global.sum_loss_since_last_dump = 0.0;
      global.old_weighted_examples = global.weighted_examples;
      global.dump_interval *= 2;
    }
}

void output_and_account_example(example* ec)
{
  global.example_number++;
  label_data* ld = (label_data*)ec->ld;
  global.weighted_examples += ld->weight;
  global.weighted_labels += ld->label * ld->weight;
  global.total_features += ec->num_features;
  global.sum_loss += ec->loss;
  global.sum_loss_since_last_dump += ec->loss;
  
  //  global.print(global.raw_prediction, ec->partial_prediction, -1, ec->tag);
  
  for (size_t i = 0; i<global.final_prediction_sink.index(); i++)
    {
      int f = global.final_prediction_sink[i].fd;
      float w;
      if (global.reg->weight_vectors != NULL) {
	w = global.reg->weight_vectors[0][global.final_prediction_sink[i].id];
      } else {
	w = 0.;
      }
      global.print(f, ec->final_prediction, w*ec->global_weight, ec->tag);
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
  
  if ( thread_num == 0 ) 
    prediction += weights[constant & thread_mask];
  
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
    prediction += weights[(constant+offset) & thread_mask];

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

  cout << "\tConstant:0:1:" << weights[(constant+offset) & global.thread_mask] << endl;
}

void print_audit_features(regressor &reg, example* ec, size_t offset)
{
  print_result(fileno(stdout),ec->final_prediction,-1,ec->tag);
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
	weights[constant & thread_mask] += update;
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
	weights[(constant+offset) & thread_mask] += update;
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

  ec->final_prediction = 
    finalize_prediction(ec->partial_prediction);

  if (ld->label != FLT_MAX)
    {
      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
      
      ec->eta_round = reg.loss->getUpdate(ec->final_prediction, ld->label, vars.eta/pow(ec->example_t,vars.power_t), ec->total_sum_feat_sq, ld->weight);
    }
  if (global.delayed_global && global.local_prediction > 0)
    ec->eta_round = 0;

  if (global.local_prediction > 0)
    {
      prediction pred={0};
      pred.p = ec->final_prediction;
      if (global.training && ld->label != FLT_MAX  && global.backprop)
        pred.p += ec->eta_round * ec->total_sum_feat_sq;
      //      cout << "sending " << pred.p << "\t" << ec->final_prediction << endl;
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
    print_audit_features(reg, ec, 0);
}

float predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars)
{
  float prediction = inline_predict(r, ex, thread_num);
  float final_pred = 0.;

  pthread_mutex_lock(&ex->lock);

  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish != 0)
    {
      while (!ex->done)
	pthread_cond_wait(&ex->finished_sum, &ex->lock);
      final_pred = ex->final_prediction;
    }
  else // We are the last thread using this example.
    {
      local_predict(ex, global.num_threads(),vars,r);
      ex->done = true;

      pthread_cond_broadcast(&ex->finished_sum);

      if (global.training && ((label_data*)(ex->ld))->label != FLT_MAX)
	delay_example(ex,global.num_threads());
      else
	delay_example(ex,0);
      final_pred = ex->final_prediction;
    }
  pthread_mutex_unlock(&ex->lock);
  return final_pred;
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
      local_predict(ex, global.num_threads(),vars,r);
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

