/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "parser.h"
#include "example.h"

void unique_sort_features(bool audit, uint32_t parse_mask, example* ae);
int order_features(const void* first, const void* second);
void unique_features(v_array<feature>& features, int max = -1);
