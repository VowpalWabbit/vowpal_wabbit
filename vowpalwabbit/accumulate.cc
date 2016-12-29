/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
/*
This implements the allreduce function of MPI.  Code primarily by
Alekh Agarwal and John Langford, with help Olivier Chapelle.
*/

#include <iostream>
#include <sys/timeb.h>
#include <cmath>
#include <stdint.h>
#include "global_data.h"
#include "vw_allreduce.h"

using namespace std;

void add_float(float& c1, const float& c2) { c1 += c2; }

void accumulate(vw& all, weight_parameters& weights, size_t offset)
{ uint32_t length = 1 << all.num_bits; //This is size of gradient
  float* local_grad = new float[length];

  for (weight_parameters::iterator iter = weights.begin(); iter != weights.end(); ++iter)
    local_grad[iter.index() >> weights.stride_shift()] = (&(*iter))[offset];
  
  all_reduce<float, add_float>(all, local_grad, length); //TODO: modify to not use first()

  for (weight_parameters::iterator iter = weights.begin(); iter != weights.end(); ++iter)
    (&(*iter))[offset] = local_grad[iter.index() >> weights.stride_shift()];

  delete[] local_grad;
}

float accumulate_scalar(vw& all, float local_sum)
{ float temp = local_sum;
  all_reduce<float, add_float>(all, &temp, 1);
  return temp;
}

void accumulate_avg(vw& all, weight_parameters& weights, size_t offset)
{ uint32_t length = 1 << all.num_bits; //This is size of gradient
  float numnodes = (float)all.all_reduce->total;
  float* local_grad = new float[length];

  for (weight_parameters::iterator iter = weights.begin(); iter != weights.end(); ++iter)
    local_grad[iter.index() >> weights.stride_shift()] = (&(*iter))[offset];
  
  all_reduce<float, add_float>(all, local_grad, length); //TODO: modify to not use first()

  for (weight_parameters::iterator iter = weights.begin(); iter != weights.end(); ++iter)
    (&(*iter))[offset] = local_grad[iter.index() >> weights.stride_shift()]/numnodes;
 
  delete[] local_grad;
}

float max_elem(float* arr, int length)
{ float max = arr[0];
  for(int i = 1; i < length; i++)
    if(arr[i] > max) max = arr[i];
  return max;
}

float min_elem(float* arr, int length)
{ float min = arr[0];
  for(int i = 1; i < length; i++)
    if(arr[i] < min && arr[i] > 0.001) min = arr[i];
  return min;
}

void accumulate_weighted_avg(vw& all, weight_parameters& weights)
{ if(!all.adaptive)
  { cerr<<"Weighted averaging is implemented only for adaptive gradient, use accumulate_avg instead\n";
    return;
  }
  uint32_t length = 1 << all.num_bits; //This is the number of parameters
  float* local_weights = new float[length];

  for (weight_parameters::iterator iter = weights.begin(); iter != weights.end(); ++iter)
    local_weights[iter.index() >> weights.stride_shift()] = (&(*iter))[1];

  //First compute weights for averaging
  all_reduce<float, add_float>(all, local_weights, length);

  for (weight_parameters::iterator iter = weights.begin(); iter != weights.end(); ++iter)
  {
    uint64_t i = iter.index() >> weights.stride_shift();
    if (local_weights[i] > 0)
	{  float ratio = (&(*iter))[1] / local_weights[i];
		local_weights[i] = *iter * ratio;
		*iter *= ratio;
		(&(*iter))[1] *= ratio; //A crude max
		if (all.normalized_updates)
			(&(*iter))[all.normalized_idx] *= ratio; //A crude max
	  }
	  else
	  {  local_weights[i] = 0;
		 *iter = 0;
	  }
  }
  all_reduce<float, add_float>(all, weights.first(), length*weights.stride_shift()); 
  delete[] local_weights;
}

