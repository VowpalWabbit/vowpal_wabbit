/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef MULTICLASS_H
#define MULTICLASS_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "example.h"

namespace MULTICLASS
{

  struct multiclass {
    uint32_t label;
    float weight;
    uint32_t prediction;
  };
  
  extern label_parser mc_label;
  
  void output_example(vw& all, example& ec);

  inline int label_is_test(multiclass* ld)
  { return ld->label == (uint32_t)-1; }

  inline int example_is_test(example* ec)
  { return label_is_test((multiclass*)ec->ld); }

  inline uint32_t get_example_label(example* ec)
  { return ((multiclass*)ec->ld)->label; }
}

#endif














