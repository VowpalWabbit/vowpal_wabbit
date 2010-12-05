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

float decayfunc4(double t, double old_t, double power_t)
{
  if (power_t > 0.99)
    return decayfunc3(t, old_t, power_t);
  else
    return decayfunc2(t, old_t, power_t);
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

void lda_loop(float* v,weight* weights,example* ec, float power_t)
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

  do
    {
      memcpy(v,new_gamma,sizeof(float)*global.lda);
      expdigammify(v);

      memcpy(old_gamma,new_gamma,sizeof(float)*global.lda);
      for (size_t k = 0; k<global.lda; k++)
	new_gamma[k] = 0;

      size_t word_count = 0;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	{
	  feature *f = ec->subsets[*i][0];
	  for (; f != ec->subsets[*i][1]; f++)
	    {
	      float* u_for_w = &weights[f->weight_index+global.lda+1];
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
    }
  while (average_diff(old_gamma, new_gamma) > 0.1);

  ec->topic_predictions.erase();
  if (ec->topic_predictions.end_array - ec->topic_predictions.begin < (int)global.lda)
    reserve(ec->topic_predictions,global.lda);
  memcpy(ec->topic_predictions.begin,new_gamma,global.lda*sizeof(float));
}

struct index_triple {
  uint32_t document;
  feature f;
};

v_array<v_array<index_triple> > merge_set;

void merge_pair(v_array<index_triple>& source, v_array<index_triple>& dest)
{
  size_t dest_size = dest.index();
  size_t limit = source.index()+dest_size;
  if (dest.end_array - dest.begin < (int)limit)
    reserve(dest,limit);
  memmove(dest.begin+source.index(),dest.begin,dest_size*sizeof(index_triple));
  dest.end = dest.begin+limit;

  size_t old_index = source.index();
  size_t new_index = 0;
  
  for (index_triple* s=source.begin; s != source.end; s++)
    {
//       fprintf(stderr, "moving dest to dest\n");
      while((old_index < limit) && (dest[old_index].f.weight_index < s->f.weight_index)) {
//         fprintf(stderr, "moving dest[%d] to dest[%d]  ---  %d to %d\n", old_index, new_index, dest[old_index].f.weight_index/global.stride, dest[new_index].f.weight_index/global.stride);
	dest[new_index++] = dest[old_index++];
      }
//       for (index_triple* i = dest.begin; i != dest.begin + new_index; i++)
//         fprintf(stderr, "%d \t %d\n", (i-dest.begin), i->f.weight_index/global.stride);
//       fprintf(stderr, "moving s to dest[%d]  ---  %d to %d\n", new_index, s->f.weight_index/global.stride, dest[new_index].f.weight_index/global.stride);
      dest[new_index++] = *s;
    }
  source.erase();
}

void merge()
{
  for (size_t j = merge_set.index()-1; j > 0;j--)
    if (merge_set[j].index()*2 > merge_set[j-1].index())
      {
	merge_pair(merge_set[j], merge_set[j-1]);
	merge_set.pop();
      }
    else
      break;
}

void merge_all()
{
  for (size_t j = merge_set.index()-1; j > 0;j--)
    {
      merge_pair(merge_set[j], merge_set[j-1]);
      merge_set.pop();
    }
}

void bubble_sort(feature* f0, feature*f1) {
  for (; f0 < f1; f1--)
    for (feature* f = f0+1; f < f1; f++)
      if (f->weight_index < (f-1)->weight_index) {
        feature temp = *f;
        *f = *(f-1);
        *(f-1) = temp;
      }
}

// TODO: this doesn't work, because the features don't come in in order.
void merge_in(example* ec, size_t document)
{
  size_t next_index = merge_set.index();
  if ((int)(next_index + ec->indices.index()) > merge_set.end_array-merge_set.begin)
    reserve(merge_set,next_index+ec->indices.index());
  merge_set.end = merge_set.begin+next_index+ec->indices.index();
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    {
      feature* f = ec->subsets[*i][0];
      //bubble_sort(f, ec->subsets[*i][1]); use --sort_features
      for (; f != ec->subsets[*i][1]; f++)
	{
	  index_triple temp = {document,*f};
	  push(merge_set[next_index], temp);
	}
      next_index++;
    }

  merge();
}

void start_lda(gd_thread_params t)
{
  regressor reg = t.reg;
  example* ec = NULL;

  float power_t = t.vars->power_t;
  double total_lambda[global.lda];
  for (size_t k = 0; k < global.lda; k++)
    total_lambda[k] = 0;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];

  for (size_t i =0; i <= global.thread_mask;i+=stride)
    for (size_t k = 0; k < global.lda; k++)
      total_lambda[k] += weights[i+k];

  double example_t = 0;
  while ( true )
    {
      float total_new[global.lda];
      for (size_t k = 0; k < global.lda; k++)
	total_new[k] = 0.f;

      while (merge_set.index() > 0)
	{
	  merge_set[merge_set.index()-1].erase();
	  merge_set.pop();
	}

      float eta = -1;
      float minuseta = -1;
      example* examples[global.minibatch];
      size_t batch_size = global.minibatch;
      for (size_t d = 0; d < batch_size; d++)
	{
	  if ((ec = get_example(0)) != NULL)//semiblocking operation.
	    {
	      examples[d] = ec;
	      merge_in(ec,d);
	    }
	  else if (thread_done(0))
	    batch_size = d;
	  else
	    d--;
	}

      merge_all(); //Now merge_set[0] contains everything.

//       fprintf(stderr, "merge_set[0].index() = %d\n", merge_set[0].index());
//       for (int i = 0; i < merge_set[0].index(); i++) {
//         fprintf(stderr, "merge_set[0][%d] = %d --- %d\n", i,
//                 merge_set[0][i].f.weight_index/global.stride, merge_set[0][i].document);
//       }

      example_t = (examples[0]->example_t - global.initial_t)/global.minibatch + global.initial_t;
      eta = (global.eta*global.lda_D) / 
	(pow(example_t, t.vars->power_t)*batch_size);
      minuseta = decayfunc4(example_t, example_t-1, power_t);
      
      float digammas[global.lda];
      float additional = (float)(global.length()) * global.lda_rho;
      for (size_t i = 0; i<global.lda; i++)
	digammas[i] = mydigamma(total_lambda[i] + additional);
      
      size_t last_weight_index = -1;
      for (index_triple* s = merge_set[0].begin; s != merge_set[0].end; s++)
	{
	  if (last_weight_index == s->f.weight_index)
	    continue;
	  last_weight_index = s->f.weight_index;
	  float* weights_for_w = &(weights[s->f.weight_index & global.thread_mask]);
	  float olddecay = decayfunc4(example_t-1, weights_for_w[global.lda], power_t);
	  float decay = decayfunc4(example_t, weights_for_w[global.lda], power_t);
	  float* u_for_w = weights_for_w + global.lda+1;
	    
	  weights_for_w[global.lda] = example_t;
	  for (size_t k = 0; k < global.lda; k++)
	    {
	      total_lambda[k] -= weights_for_w[k] * olddecay;
	      weights_for_w[k] *= decay;
	      u_for_w[k] = weights_for_w[k] + global.lda_rho;
	    }
	  expdigammify_2(u_for_w, digammas);
	}

      float v[batch_size*global.lda];
      
      for (size_t d = 0; d < batch_size; d++)
	lda_loop(&v[d*global.lda],weights,examples[d],t.vars->power_t);

      for (index_triple* s = merge_set[0].begin; s != merge_set[0].end;)
	{
	  index_triple* next = s+1;
	  while(next != merge_set[0].end && next->f.weight_index == s->f.weight_index)
	    next++;

	  float* word_weights = &(weights[s->f.weight_index & global.thread_mask]);
	  for (size_t k = 0; k < global.lda; k++) {
	    float new_value = minuseta*word_weights[k];
	    total_new[k] += new_value;
	    word_weights[k] = new_value;
	  }

	  for (; s != next; s++) {
	    float* v_s = &v[s->document*global.lda];
	    float* u_for_w = &weights[(s->f.weight_index & global.thread_mask) + global.lda + 1];
	    float c_w = eta*find_cw(u_for_w, v_s)*s->f.x;
	    for (size_t k = 0; k < global.lda; k++) {
	      float new_value = u_for_w[k]*v_s[k]*c_w;
	      total_new[k] += new_value;
	      word_weights[k] += new_value;
	    }
	  }
	}
      for (size_t k = 0; k < global.lda; k++) {
	total_lambda[k] *= minuseta;
	total_lambda[k] += total_new[k];
      }

      for (size_t d = 0; d < batch_size; d++)
	{
	  if (global.audit)
	    print_audit_features(reg, examples[d]);
	  finish_example(examples[d]);
	}
      if (thread_done(0))
	{
	  for (size_t i = 0; i < global.length(); i++) {
	    weight* weights_for_w = & (weights[i*global.stride]);
	    float decay = decayfunc4(example_t, weights_for_w[global.lda], power_t);
	    for (size_t k = 0; k < global.lda; k++)
	      weights_for_w[k] *= decay;
	  }

	  for (size_t k = 0; k < global.lda; k++) {
	    for (size_t i = 0; i < global.length(); i++) {
	      fprintf(stdout, "%0.3f ", weights[i*global.stride + k] + global.lda_rho);
	    }
	    fprintf(stdout, "\n");
	  }
	  
	  if (global.local_prediction > 0)
	    shutdown(global.local_prediction, SHUT_WR);

	  for (int i = 0; i < merge_set.end_array-merge_set.begin; i++)
	    free(merge_set[i].begin);
	  free(merge_set.begin);
	  return;
	}
    }
}

void end_lda()
{
  
}
