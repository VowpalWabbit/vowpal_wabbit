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
}

int read_features_string(vw* all, v_array<example*>& examples);
size_t read_features(vw* all, char*& line, size_t& num_chars);

template<bool audit>
int read_features_json(vw* all, v_array<example*>& examples)
{
	char* line;
	size_t num_chars;
	size_t num_chars_initial = read_features(all, line, num_chars);
	if (num_chars_initial < 1)
		return (int)num_chars_initial;

	line[num_chars] = '\0';
	VW::read_line_json<audit>(*all, examples, line, reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), all);

	// note: the json parser does single pass parsing and cannot determine if a shared example is needed.
	// since the communication between the parsing thread the main learner expects examples to be requested in order (as they're layed out in memory)
	// there is no way to determine upfront if a shared example exists
	// thus even if there are no features for the shared example, still an empty example is returned.

	if (examples.size() > 1)
	{ // insert new line example at the end
		example& ae = VW::get_unused_example(all);
		char empty = '\0';
		substring example = { &empty, &empty };
		substring_to_example(all, &ae, example);

		examples.push_back(&ae);
	}

	return 1;
}
