/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "parser.h"
#include "example.h"

void unique_sort_features(bool audit, uint64_t parse_mask, example* ae);

inline int order_features(const void* first, const void* second)
{ if (((feature_slice*)first)->i != ((feature_slice*)second)->i)
    return (int)(((feature_slice*)first)->i - ((feature_slice*)second)->i);
  else if (((feature_slice*)first)->v > ((feature_slice*)second)->v)
    return 1;
  else
    return -1;
}

void unique_features(features& fs, int max = -1);
