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
example* get_unused_example(vw* all);
void read_line(vw& all, example* ex, char* line);//read example from the line.

// can't type as it forces C++/CLI part to include rapidjson, which leads to name clashes...
typedef example* (*example_factory_t)(void*);

void read_line_json(vw& all, v_array<example*>& examples, char* line, example_factory_t example_factory, void* ex_factory_context);

}

template<FeatureInputType ft>
int read_features(vw* all, v_array<example*>& examples)
{ example* ae = examples[0];

  char *line=nullptr;
  size_t num_chars_initial = readto(*(all->p->input), line, '\n');
  if (num_chars_initial < 1)
    return (int)num_chars_initial;
  size_t num_chars = num_chars_initial;
  if (line[0] =='\xef' && num_chars >= 3 && line[1] == '\xbb' && line[2] == '\xbf')
  { line += 3;
    num_chars -= 3;
  }
  if (line[num_chars-1] == '\n')
    num_chars--;
  if (line[num_chars-1] == '\r')
    num_chars--;

  if (ft == StringFeatures)
  { substring example = { line, line + num_chars };
	substring_to_example(all, ae, example);
	  
	return (int)num_chars_initial;
  }
  else if (ft == JsonFeatures)
  { VW::read_line_json(*all, examples, line, reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), all);
	return 1;
  }
  else
	  assert(false);
}

