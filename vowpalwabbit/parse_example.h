/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <stdint.h>
#include "parse_primitives.h"
#include "example.h"

//example processing
typedef enum
{
	StringFeatures,
	JsonFeatures
} FeatureInputType;

void substring_to_example(vw* all, example* ae, substring example);

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
  {
	  substring example = { line, line + num_chars };
	  substring_to_example(all, ae, example);
	  
	  return (int)num_chars_initial;
  }
  else if (ft == JsonFeatures)
  {
	  return 0;
  }
  else
	  assert(false);
}
namespace VW
{
void read_line(vw& all, example* ex, char* line);//read example from the line.
}
