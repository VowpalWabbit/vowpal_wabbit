/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <stdint.h>
#include "parse_primitives.h"
#include "example.h"
#include "vw.h"

//example processing
typedef enum
{
	StringFeatures,
	JsonFeatures
} FeatureInputType;

void substring_to_example(vw* all, example* ae, substring example);

namespace VW
{
example& get_unused_example(vw* all);
void read_line(vw& all, example* ex, char* line);//read example from the line.

void read_line_json(vw& all, v_array<example*>& examples, char* line, example_factory_t example_factory, void* ex_factory_context);
}

int read_features_string(vw* all, v_array<example*>& examples);
int read_features_json(vw* all, v_array<example*>& examples);
