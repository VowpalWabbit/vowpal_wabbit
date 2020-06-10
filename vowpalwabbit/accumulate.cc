// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

/*
This implements the allreduce function of MPI.  Code primarily by
Alekh Agarwal and John Langford, with help Olivier Chapelle.
*/

#include <iostream>
#include <cmath>
#include <cstdint>
#include "global_data.h"
#include "vw_allreduce.h"

void add_float(float& c1, const float& c2) { c1 += c2; }

void accumulate(vw& all, parameters& weights, size_t offset)
{
  uint64_t length = UINT64_ONE << all.num_bits;  // This is size of gradient
  float* local_grad = new float[length];

  if (weights.sparse)
    for (uint64_t i = 0; i < length; i++)
      local_grad[i] = (&(weights.sparse_weights[i << weights.sparse_weights.stride_shift()]))[offset];
  else
    for (uint64_t i = 0; i < length; i++)
      local_grad[i] = (&(weights.dense_weights[i << weights.dense_weights.stride_shift()]))[offset];

  all_reduce<float, add_float>(all, local_grad, length);  // TODO: modify to not use first()

  if (weights.sparse)
    for (uint64_t i = 0; i < length; i++)
      (&(weights.sparse_weights[i << weights.sparse_weights.stride_shift()]))[offset] = local_grad[i];
  else
    for (uint64_t i = 0; i < length; i++)
      (&(weights.dense_weights[i << weights.dense_weights.stride_shift()]))[offset] = local_grad[i];

  delete[] local_grad;
}

float accumulate_scalar(vw& all, float local_sum)
{
  float temp = local_sum;
  all_reduce<float, add_float>(all, &temp, 1);
  return temp;
}

void accumulate_avg(vw& all, parameters& weights, size_t offset)
{
  uint32_t length = 1 << all.num_bits;  // This is size of gradient
  float numnodes = (float)all.all_reduce->total;
  float* local_grad = new float[length];

  if (weights.sparse)
    for (uint64_t i = 0; i < length; i++)
      local_grad[i] = (&(weights.sparse_weights[i << weights.sparse_weights.stride_shift()]))[offset];
  else
    for (uint64_t i = 0; i < length; i++)
      local_grad[i] = (&(weights.dense_weights[i << weights.dense_weights.stride_shift()]))[offset];

  all_reduce<float, add_float>(all, local_grad, length);  // TODO: modify to not use first()

  if (weights.sparse)
    for (uint64_t i = 0; i < length; i++)
      (&(weights.sparse_weights[i << weights.sparse_weights.stride_shift()]))[offset] = local_grad[i] / numnodes;
  else
    for (uint64_t i = 0; i < length; i++)
      (&(weights.dense_weights[i << weights.dense_weights.stride_shift()]))[offset] = local_grad[i] / numnodes;

  delete[] local_grad;
}

float max_elem(float* arr, int length)
{
  float max = arr[0];
  for (int i = 1; i < length; i++)
    if (arr[i] > max)
      max = arr[i];
  return max;
}

float min_elem(float* arr, int length)
{
  float min = arr[0];
  for (int i = 1; i < length; i++)
    if (arr[i] < min && arr[i] > 0.001)
      min = arr[i];
  return min;
}

template <class T>
void do_weighting(vw& all, uint64_t length, float* local_weights, T& weights)
{
  for (uint64_t i = 0; i < length; i++)
  {
    float* weight = &weights[i << weights.stride_shift()];
    if (local_weights[i] > 0)
    {
      float ratio = weight[1] / local_weights[i];
      local_weights[i] = weight[0] * ratio;
      weight[0] *= ratio;
      weight[1] *= ratio;  // A crude max
      if (all.normalized_idx > 0)
        weight[all.normalized_idx] *= ratio;  // A crude max
    }
    else
    {
      local_weights[i] = 0;
      *weight = 0;
    }
  }
}

void accumulate_weighted_avg(vw& all, parameters& weights)
{
  if (!weights.adaptive)
  {
    all.trace_message << "Weighted averaging is implemented only for adaptive gradient, use accumulate_avg instead\n";
    return;
  }

  uint32_t length = 1 << all.num_bits;  // This is the number of parameters
  float* local_weights = new float[length];

  if (weights.sparse)
    for (uint64_t i = 0; i < length; i++)
      local_weights[i] = (&(weights.sparse_weights[i << weights.sparse_weights.stride_shift()]))[1];
  else
    for (uint64_t i = 0; i < length; i++)
      local_weights[i] = (&(weights.dense_weights[i << weights.dense_weights.stride_shift()]))[1];

  // First compute weights for averaging
  all_reduce<float, add_float>(all, local_weights, length);

  if (weights.sparse)
    do_weighting(all, length, local_weights, weights.sparse_weights);
  else
    do_weighting(all, length, local_weights, weights.dense_weights);

  if (weights.sparse)
    std::cout << "sparse parameters not supported with parallel computation!" << std::endl;
  else
    all_reduce<float, add_float>(
        all, weights.dense_weights.first(), ((size_t)length) * (1ull << weights.stride_shift()));
  delete[] local_weights;
}
