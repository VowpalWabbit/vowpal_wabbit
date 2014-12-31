/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "global_data.h"

vw* parse_args(int argc, char *argv[]);
LEARNER::base_learner* setup_base(vw& all, po::variables_map& vm);
