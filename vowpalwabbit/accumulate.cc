/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

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

struct timeb t_start, t_end;
double net_comm_time = 0.;

void accumulate(string master_location, regressor& reg, size_t o) {
  ftime(&t_start);
  uint32_t length = 1 << global.num_bits; //This is size of gradient
  size_t stride = global.stride;
  float* local_grad = new float[length];
  weight* weights = reg.weight_vectors;
  for(uint32_t i = 0;i < length;i++) 
    {
      local_grad[i] = weights[stride*i+o];
    }

  all_reduce(local_grad, length, master_location, global.unique_id, global.total, global.node);
  for(uint32_t i = 0;i < length;i++) 
    {
      weights[stride*i+o] = local_grad[i];
    }
  delete[] local_grad;
  ftime(&t_end);
  net_comm_time += (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
}

float accumulate_scalar(string master_location, float local_sum) {
  ftime(&t_start);
  float temp = local_sum;
  all_reduce(&temp, 1, master_location, global.unique_id, global.total, global.node);
  ftime(&t_end);
  net_comm_time += (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
  return temp;
}

void accumulate_avg(string master_location, regressor& reg, size_t o) {
  uint32_t length = 1 << global.num_bits; //This is size of gradient
  size_t stride = global.stride;
  float* local_grad = new float[length];
  weight* weights = reg.weight_vectors;
  ftime(&t_start);
  float numnodes = 1.;
  all_reduce(&numnodes, 1, master_location, global.unique_id, global.total, global.node);
  for(uint32_t i = 0;i < length;i++) 
    {
      local_grad[i] = weights[stride*i+o];
    }

  all_reduce(local_grad, length, master_location, global.unique_id, global.total, global.node);
  for(uint32_t i = 0;i < length;i++) 
    {
      weights[stride*i+o] = local_grad[i]/numnodes;
    }
  ftime(&t_end);
  net_comm_time += (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
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

void accumulate_weighted_avg(string master_location, regressor& reg) {
  if(!global.adaptive) {
    cerr<<"Weighted averaging is implemented only for adaptive gradient, use accumulate_avg instead\n";
    return;
  }
  uint32_t length = 1 << global.num_bits; //This is size of gradient
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors;
  float* local_weights = new float[length];

  ftime(&t_start);
  for(uint32_t i = 0;i < length;i++) 
    local_weights[i] = sqrt(weights[stride*i+1]*weights[stride*i+1]-1);
  
  all_reduce(local_weights, length, master_location, global.unique_id, global.total, global.node);

  for(uint32_t i = 0;i < length;i++) 
    if(local_weights[i] > 0) {
      float ratio = sqrt(weights[stride*i+1]*weights[stride*i+1]-1)/local_weights[i];
      weights[stride*i] *= ratio;
      weights[stride*i+1] *= ratio;
    }
    else 
      weights[stride*i] = 0; 

  all_reduce(weights, 2*length, master_location, global.unique_id, global.total, global.node);

  ftime(&t_end);
  net_comm_time += (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
  delete[] local_weights;
}

double get_comm_time() {
  return net_comm_time;
}
