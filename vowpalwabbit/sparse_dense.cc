/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */

#include "sparse_dense.h"
#include "constant.h"
#include <math.h>


void sd_offset_update(weight* weights, size_t mask, feature* begin, feature* end, size_t offset, float update, float regularization)
{
  for (feature* f = begin; f!= end; f++) 
    weights[(f->weight_index + offset) & mask] += update * f->x - regularization * weights[(f->weight_index + offset) & mask];
} 

