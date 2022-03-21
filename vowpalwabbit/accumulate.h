// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This implements various accumulate functions building on top of allreduce.
#pragma once

#include "vw_fwd.h"

#include <cstddef>

void accumulate(VW::workspace& all, parameters& weights, size_t o);
float accumulate_scalar(VW::workspace& all, float local_sum);
void accumulate_weighted_avg(VW::workspace& all, parameters& weights);
void accumulate_avg(VW::workspace& all, parameters& weights, size_t o);
