/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/

#include "example_predict.h"

safe_example_predict::safe_example_predict()
{
  indices = v_init<namespace_index>();
  // feature_space is initialized through constructors
}

safe_example_predict::~safe_example_predict()
{
  indices.delete_v();
  for (size_t i = 0; i < 256; i++) feature_space[i].delete_v();
}
