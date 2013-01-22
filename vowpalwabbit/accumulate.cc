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
#include "accumulate.h"
#include "global_data.h"
   
using namespace std;

void accumulate(vw& all, string master_location, regressor& reg, size_t o) {
  uint32_t length = 1 << all.num_bits; //This is size of gradient
  size_t stride = all.stride;
  float* local_grad = new float[length];
  weight* weights = reg.weight_vector;
  for(uint32_t i = 0;i < length;i++) 
    {
      local_grad[i] = weights[stride*i+o];
    }

  all_reduce(local_grad, length, master_location, all.unique_id, all.total, all.node, all.socks);
  for(uint32_t i = 0;i < length;i++) 
    {
      weights[stride*i+o] = local_grad[i];
    }
  delete[] local_grad;
}

float accumulate_scalar(vw& all, string master_location, float local_sum) {
  float temp = local_sum;
  all_reduce(&temp, 1, master_location, all.unique_id, all.total, all.node, all.socks);
  return temp;
}

void accumulate_avg(vw& all, string master_location, regressor& reg, size_t o) {
  uint32_t length = 1 << all.num_bits; //This is size of gradient
  size_t stride = all.stride;
  float* local_grad = new float[length];
  weight* weights = reg.weight_vector;
  float numnodes = 1.;
  all_reduce(&numnodes, 1, master_location, all.unique_id, all.total, all.node, all.socks);
  for(uint32_t i = 0;i < length;i++) 
    {
      local_grad[i] = weights[stride*i+o];
    }

  all_reduce(local_grad, length, master_location, all.unique_id, all.total, all.node, all.socks);
  for(uint32_t i = 0;i < length;i++) 
    {
      weights[stride*i+o] = local_grad[i]/numnodes;
    }
  delete[] local_grad;
}

float max_elem(float* arr, int length) {
  float max = arr[0];
  for(int i = 1;i < length;i++)
    if(arr[i] > max) max = arr[i];
  return max;
}

float min_elem(float* arr, int length) {
  float min = arr[0];
  for(int i = 1;i < length;i++)
    if(arr[i] < min && arr[i] > 0.001) min = arr[i];
  return min;
}

void accumulate_weighted_avg(vw& all, string master_location, regressor& reg) {
  if(!all.adaptive) {
    cerr<<"Weighted averaging is implemented only for adaptive gradient, use accumulate_avg instead\n";
    return;
  }
  uint32_t length = 1 << all.num_bits; //This is size of gradient
  size_t stride = all.stride;
  weight* weights = reg.weight_vector;
  float* local_weights = new float[length];

  for(uint32_t i = 0;i < length;i++) 
    local_weights[i] = sqrt(weights[stride*i+1]*weights[stride*i+1]-1);
  
  all_reduce(local_weights, length, master_location, all.unique_id, all.total, all.node, all.socks);

  for(uint32_t i = 0;i < length;i++) 
    if(local_weights[i] > 0) {
      float ratio = sqrt(weights[stride*i+1]*weights[stride*i+1]-1)/local_weights[i];
      weights[stride*i] *= ratio;
      weights[stride*i+1] *= ratio;
    }
    else 
      weights[stride*i] = 0; 

  all_reduce(weights, 2*length, master_location, all.unique_id, all.total, all.node, all.socks);

  delete[] local_weights;
}

