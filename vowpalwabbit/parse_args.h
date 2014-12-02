/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "global_data.h"

vw* parse_args(int argc, char *argv[]);
LEARNER::learner* setup_next(vw& all, po::variables_map& vm);
