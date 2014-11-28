/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "label_parser.h"

struct example;
struct vw;

namespace MULTICLASS
{
  struct multiclass {
    uint32_t label;
    float weight;
  };
  
  extern label_parser mc_label;
  
  void output_example(vw& all, example& ec);

  inline int label_is_test(multiclass* ld)
  { return ld->label == (uint32_t)-1; }
}
