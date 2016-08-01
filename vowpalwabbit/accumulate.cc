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

void accumulate(vw& all, weight_parameters& weights, size_t o)
{ uint32_t length = 1 << all.num_bits; //This is size of gradient
  weight_parameters local_grad(length);
 
  weight_parameters::iterator i = weights.begin(o);
  weight_parameters::iterator j = local_grad.begin();
  for (; i != weights.end(o); ++i, ++j)
	  *j = *i;

  all_reduce<float, add_float>(all, local_grad.first(), length); //TODO: modify to not use first()
 
  i = weights.begin(o);
  j = local_grad.begin();
  for (; i != weights.end(o); ++i, ++j)
	  *i = *j;
}

float accumulate_scalar(vw& all, float local_sum)
{ float temp = local_sum;
  all_reduce<float, add_float>(all, &temp, 1);
  return temp;
}

void accumulate_avg(vw& all, weight_parameters& weights, size_t o)
{ uint32_t length = 1 << all.num_bits; //This is size of gradient
  float numnodes = (float)all.all_reduce->total;
  weight_parameters local_grad(length);

  weight_parameters::iterator i = weights.begin(o);
  weight_parameters::iterator j = local_grad.begin();
  for (; i != weights.end(); ++i, ++j)
	  *j = *i;

  all_reduce<float, add_float>(all, local_grad.first(), length); 
 
  i = weights.begin(o);
  j = local_grad.begin();
  for (; i != weights.end(); ++i, ++j)
	  *i = *j/numnodes;
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
  weight_parameters local_weights(length);

  weight_parameters::iterator i = weights.begin(1);
  weight_parameters::iterator j = local_weights.begin();
  for (; i != weights.end(1); ++i, ++j)
	  *j = *i;

  //First compute weights for averaging
  all_reduce<float, add_float>(all, local_weights.first(), length); 

  weight_parameters::iterator weights_0 = weights.begin(0); 
  weight_parameters::iterator weights_1 = weights.begin(1); 
  weight_parameters::iterator weights_normal_idx = weights.begin(all.normalized_idx);
  weight_parameters::iterator local = local_weights.begin();

  for (; weights_0 != weights.end(); ++weights_0, ++weights_1, ++weights_normal_idx, ++local)
	if (*local > 0)
	 {  float ratio = *weights_1 / *local;
		*local = *weights_0 * ratio;
		*weights_0 *= ratio;
		*weights_1 *= ratio; //A crude max
		if (all.normalized_updates)
		  *weights_normal_idx *= ratio; //A crude max
	  }
	  else
	  {  *local = 0;
		 *weights_0 = 0;
	  }
	  
  all_reduce<float, add_float>(all, weights.first(), length*all.stride_shift); 

}

