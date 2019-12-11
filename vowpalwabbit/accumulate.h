// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This implements various accumulate functions building on top of allreduce.
#pragma once
#include "global_data.h"

void accumulate(vw& all, parameters& weights, size_t o);
float accumulate_scalar(vw& all, float local_sum);
void accumulate_weighted_avg(vw& all, parameters& weights);
void accumulate_avg(vw& all, parameters& weights, size_t o);
