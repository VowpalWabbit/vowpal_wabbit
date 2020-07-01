// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include <cstdint>
#include "parse_primitives.h"
#include "example.h"
#include "vw.h"
#include "vw_string_view.h"

// example processing
typedef enum
{
  StringFeatures,
  JsonFeatures
} FeatureInputType;

void substring_to_example(vw* all, example* ae, VW::string_view example);

namespace VW
{
example& get_unused_example(vw* all);
void read_line(vw& all, example* ex, char * line);  // read example from the line.
void read_lines(vw* all, char* line, size_t len,
    v_array<example*>& examples);  // read examples from the new line separated strings.

}  // namespace VW

int read_features_string(vw* all, v_array<example*>& examples);
size_t read_features(vw* all, char*& line, size_t& num_chars);
