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
#include <boost/math/special_functions/digamma.hpp>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "lda.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"

using namespace boost::math::policies;

size_t max_w = 0;

float mydigamma(float x) 
{
  return boost::math::digamma(x);
}

float decayfunc(float t, float old_t, float power_t) {
  float result = 1;
  for (float i = old_t+1; i <= t; i += 1)
    result *= (1-pow(i, -power_t));
  return result;
}

float decayfunc2(float t, float old_t, float power_t) 
{
  float power_t_plus_one = 1. - power_t;
  float arg =  - ( pow(t, power_t_plus_one) -
                   pow(old_t, power_t_plus_one));
  return exp ( arg
               / power_t_plus_one);
}

float decayfunc3(double t, double old_t, double power_t) 
{
  double power_t_plus_one = 1. - power_t;
  double logt = log(t);
  double logoldt = log(old_t);
  return (old_t / t) * exp(0.5*power_t_plus_one*(-logt*logt + logoldt*logoldt));
}

void expdigammify(float* gamma)
{
  float sum=0;
  for (size_t i = 0; i<global.lda; i++)
    {
      sum += gamma[i];
      gamma[i] = mydigamma(gamma[i]);
    }
  sum = mydigamma(sum);
  for (size_t i = 0; i<global.lda; i++)
    gamma[i] = exp(gamma[i] - sum);
}

void expdigammify_2(float* gamma, float* norm)
{
  for (size_t i = 0; i<global.lda; i++)
    {
      gamma[i] = exp(mydigamma(gamma[i]) - norm[i]);
    }
}

float average_diff(float* oldgamma, float* newgamma)
{
  float sum = 0.;
  for (size_t i = 0; i<global.lda; i++)
    sum += fabsf(oldgamma[i] - newgamma[i]);
  return sum;
}

float find_cw(float* u_for_w, float* v)
{
  float c_w = 0;
  for (size_t k =0; k<global.lda; k++)
    c_w += u_for_w[k]*v[k];

  return 1.f / c_w;
}

void lda_loop(float* v,v_array<float>& u_kw,double* total_lambda,weight* weights,example* ec, float power_t)
{
  float new_gamma[global.lda];
  float old_gamma[global.lda];
  
  for (size_t i = 0; i < global.lda; i++)
    {
      new_gamma[i] = 1.;
      old_gamma[i] = 0.;
    }
  size_t num_words =0;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    num_words += ec->subsets[*i][1] - ec->subsets[*i][0];

  u_kw.erase();
  reserve(u_kw,num_words*global.lda);
  
  float digammas[global.lda];
  float additional = (float)(global.length()) * global.lda_rho;
  for (size_t i = 0; i<global.lda; i++)
    {
      digammas[i] = mydigamma(total_lambda[i] + additional);
    }
  size_t word_count = 0;
  float words_in_document = 0;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    {
      feature *f = ec->subsets[*i][0];

      for (; f != ec->subsets[*i][1]; f++){
	words_in_document += f->x;
	float* u_for_w = u_kw.begin+(global.lda*word_count);
	float* weights_for_w = &(weights[f->weight_index & global.thread_mask]);
	float olddecay = decayfunc3(ec->example_t-1, weights_for_w[global.lda], power_t);
	float decay = decayfunc3(ec->example_t, weights_for_w[global.lda], power_t);

	weights_for_w[global.lda] = ec->example_t;
	for (size_t k = 0; k < global.lda; k++)
	  {
	    total_lambda[k] -= weights_for_w[k] * olddecay;
	    weights_for_w[k] *= decay;
	    u_for_w[k] = weights_for_w[k] + global.lda_rho;
	  }
	expdigammify_2(u_for_w, digammas);
	word_count++;
      }
    }

  size_t numits = 0;

  do
    {
      memcpy(v,new_gamma,sizeof(float)*global.lda);
      expdigammify(v);

      memcpy(old_gamma,new_gamma,sizeof(float)*global.lda);
      for (size_t k = 0; k<global.lda; k++)
	new_gamma[k] = 0;

      word_count = 0;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	{
	  feature *f = ec->subsets[*i][0];
	  for (; f != ec->subsets[*i][1]; f++)
	    {
	      float* u_for_w = &u_kw[word_count*global.lda];
	      float c_w = find_cw(u_for_w,v);
	      float xc_w = c_w * f->x;
	      size_t max_k = global.lda;
	      for (size_t k =0; k<max_k; k++) {
		new_gamma[k] += xc_w*u_for_w[k];
	      }
	      word_count++;
	    }
	}
      for (size_t k =0; k<global.lda; k++) {
	new_gamma[k] = new_gamma[k]*v[k]+global.lda_alpha;
      }
      numits++;
    }
  while (average_diff(old_gamma, new_gamma) > 0.1);

  ec->topic_predictions.erase();
  reserve(ec->topic_predictions,global.lda);
  memcpy(ec->topic_predictions.begin,new_gamma,global.lda*sizeof(float));
}

struct index_pair {
  uint32_t feature;
  uint32_t document;
};

v_array<v_array<index_pair> > merge_set;

void merge(v_array<index_pair>& source, v_array<index_pair>& dest)
{
  size_t limit = source.index()+dest.index();
  reserve(dest,limit);
  memmove(dest.begin+source.index(),dest.begin,dest.index());
  dest.end = dest.begin+limit;
  size_t old_index = source.index();
  size_t new_index = 0;
  
  for (index_pair* s=source.begin; s != source.end; s++)
    {
      while((old_index < limit) && (dest[old_index].feature < s->feature))
	dest[new_index++] = dest[old_index++];
      dest[new_index++] = *s;
    }
  source.erase();
}

void mergein(example* ec, size_t document)
{
  size_t num_words = 0;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    num_words += ec->subsets[*i][1] - ec->subsets[*i][0];

  size_t limit = num_words+merge_set[0].index();
  reserve(merge_set[0],limit);
  index_pair* ordered_set=merge_set[0].begin;
  memmove(ordered_set+num_words,ordered_set,merge_set[0].index());
  merge_set[0].end = merge_set[0].begin+limit;
  size_t old_index = num_words;
  size_t new_index = 0;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    {
      feature* f = ec->subsets[*i][0];
      for (; f != ec->subsets[*i][1]; f++)
	{
	  while(old_index < limit && ordered_set[old_index].feature < f->weight_index)
	    ordered_set[new_index++] = ordered_set[old_index++];
	  struct index_pair temp = {f->weight_index, document};
	  ordered_set[new_index++] = temp;
	}
    }
  for (size_t j = 1; j < merge_set.index();j++)
    if (merge_set[j-1].index() > merge_set[j].index())
      merge(merge_set[j-1],merge_set[j]);
    else
      break;
}

void start_lda(gd_thread_params t)
{
  regressor reg = t.reg;
  example* ec = NULL;

  double total_lambda[global.lda];
  for (size_t k = 0; k < global.lda; k++)
    total_lambda[k] = 0;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];

  for (size_t i =0; i <= global.thread_mask;i+=stride)
    for (size_t k = 0; k < global.lda; k++)
      {
	total_lambda[k] += weights[i+k];
      }

  v_array<float> u_kw;
  while ( true )
    {
      float total_new[global.lda];
      for (size_t k = 0; k < global.lda; k++)
	total_new[k] = 0.f;

      float eta = -1;
      float minuseta = -1;
      example* examples[global.minibatch];
      size_t batch_size = global.minibatch;
      for (size_t d = 0; d < batch_size; d++)
	{
	  if ((ec = get_example(0)) != NULL)//semiblocking operation.
	    examples[d] = ec;
	  else if (thread_done(0))
	    batch_size = d;
	  else 
	    d--;
	}

      

      float v[batch_size][global.lda];
      
      

	    {
	      if (d == 0) {
		size_t example_t = ec->example_t/global.minibatach;
		eta = (global.eta*global.lda_D) / 
		  (pow(example_t, t.vars->power_t)*global.minibatch);
		minuseta = decayfunc3(example_t, example_t-1, t.vars->power_t);
	      }

	      lda_loop(v,u_kw,total_lambda,weights,ec,t.vars->power_t);
	      size_t word_count=0;

	      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
		{
		  feature* f = ec->subsets[*i][0];
		  for (; f != ec->subsets[*i][1]; f++)
		    {
		      float* word_weights = &(weights[f->weight_index & global.thread_mask]);

		      float* u_for_w = &u_kw[word_count*global.lda];
		      float c_w = eta*find_cw(u_for_w,v)*f->x;

		      for (size_t k =0; k<global.lda;k++)
			{
			  float new_value = minuseta*word_weights[k]
			    + u_for_w[k]*v[k]*c_w;
			  total_new[k] += new_value;
			  word_weights[k] = new_value;
			}
		    }
		}
	      for (size_t k = 0; k < global.lda; k++) {
		total_lambda[k] *= minuseta;
		total_lambda[k] += total_new[k];
	      }

	      if (global.audit)
		print_audit_features(reg, ec);
	      finish_example(ec);
	    }
	  else if (thread_done(0))
	    {
	      for (size_t k = 0; k < global.lda; k++) {
		for (size_t i = 0; i < global.length(); i++) {
		  fprintf(stdout, "%0.3f ", weights[i*global.stride + k] + global.lda_rho);
		}
		fprintf(stdout, "\n");
	      }

	      if (global.local_prediction > 0)
		shutdown(global.local_prediction, SHUT_WR);
	      return;
	    }
	  else
	    ;//busywait when we have predicted on all examples but not yet trained on all.
	}
    }
}

void end_lda()
{
  
}
