/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <math.h>
#include "example.h"
#include "parse_regressor.h"
#include "parser.h"
#include "gd.h"

namespace MF{
  LEARNER::learner* setup(vw& all, po::variables_map& vm);
}
