// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "parser.h"
#include "example.h"

void unique_sort_features(uint64_t parse_mask, example* ae);

void unique_features(features& fs, int max = -1);
