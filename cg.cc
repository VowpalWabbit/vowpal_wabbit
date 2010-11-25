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

void quad_grad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float g)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float update = g * page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[3] += update * ele->x;
    }
}

void quad_sd_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float g)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float update = g * page_feature.x * page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[3] += update * ele->x * ele->x;
    }
}

// w[0] = weight
// w[1] = accumulated first derivative
// w[2] = step direction
// w[3] = accumulated second derivative

bool gradient_pass;

void cg_step(regressor& reg, example* &ec)
{
  float raw_prediction = inline_predict(reg,ec,0);
  float final_prediciton = finalize_prediction(raw_prediction);
  if (gradient_pass)
    {
      float loss_grad = reg.loss->getfirstderivative(ec->final_prediction,ld->label);
      
      weight* weights = reg.weight_vectors[thread_num];
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	{
	  feature *f = ec->subsets[*i][thread_num];
	  for (; f != ec->subsets[*i][thread_num+1]; f++)
	    {
	      weight* w = &weights[f->weight_index & thread_mask];
	      w[1] += loss_grad * f->x;
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
		quad_grad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, g);
	    } 
	}
    }
  else // in the second pass
    {
      float loss_sd = reg.loss->second_derivative(ec->final_prediction,ld->label);
      
      weight* weights = reg.weight_vectors[thread_num];
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	{
	  feature *f = ec->subsets[*i][thread_num];
	  for (; f != ec->subsets[*i][thread_num+1]; f++)
	    {
	      weight* w = &weights[f->weight_index & thread_mask];
	      w[1] += loss_sd * f->x * f->x;
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
		quad_sd_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, g);
	    } 
	}
    }
}

double derivative_magnitude(regressor& reg)
{
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length; i++)
    ret += r.weight_vectors[0][stride*i+1]*r.weight_vectors[0][stride*i+1];
  return ret;
}

double second_derivative_in_direction(regressor& reg)
{
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length; i++)
    ret += r.weight_vectors[0][stride*i+3]*r.weight_vectors[0][stride*i+2]*r.weight_vectors[0][stride*i+2];
  return ret;
}

double derivative_in_direction(regressor& reg)
{
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length; i++)
    ret += r.weight_vectors[0][stride*i+1]*r.weight_vectors[0][stride*i+2];
  return ret;
}

void update_direction(regressor& reg, float old_portion)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length; i++)
    r.weight_vectors[0][stride*i+2] = r.weight_vectors[0][stride*i+1] + old_portion * r.weight_vectors[0][stride*i+2];
}

void update_weight(regressor& reg, float step_size)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length; i++)
    r.weight_vectors[0][stride*i] += step_size * r.weight_vectors[0][stride*i+2];
}

void setup_cg(gd_thread_params t)
{
  gd_thread_params* params = (gd_thread_params*) in;
  regressor reg = params->reg;
  size_t thread_num = params->thread_num;
  example* ec = NULL;

  double previous_d_mag;
  gradient_pass = true;
  size_t current_pass = 0;
  while ( true )
    {//this is a poor man's select operation.
      if ((ec = get_example(thread_num)) != NULL)//semiblocking operation.
	{
	  assert(ec->in_use);
	  if (ec->pass != current_pass)//we need to do work on all features.
	    if (gradient_pass) // We just finished computing all gradients
	      {
		float mix_frac;
		if (current_pass == 0)
		  {
		    previous_d_mag = derivative_magnitude(reg);
		    mix_frac = 0;
		  }
		else
		  {
		    float new_d_mag = derivative_magnitude(reg);
		    mix_frac = new_d_mag/previous_d_mag;
		    previous_d_mag = new_d_mag;
		  }
		update_direction(reg, mix_frac);
		gradient_pass = false;
	      }
	    else // just finished all second gradients
	      {
		float step_size = - derivative_in_direction(reg)/second_derivative_in_direction(reg);
		update_weight(reg,step_size);
	      }
	  cg_step(reg,ec);
	  finish_example(ec);
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

void destroy_cg()
{
}

