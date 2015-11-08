/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "parser.h"
#include "example.h"

void unique_sort_features(bool audit, uint32_t parse_mask, example* ae);
template <class T> int order_features(const void* first, const void* second)
{ 
  if (((T*)first)->weight_index != ((T*)second)->weight_index)
    return ((T*)first)->weight_index - ((T*)second)->weight_index;
  else
    if (((T*)first)->x > ((T*)second)->x)
      return 1;
    else 
      return -1;
}
void unique_features(v_array<feature>& features, int max = -1);
