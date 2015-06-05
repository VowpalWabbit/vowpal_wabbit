/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "global_data.h"

vw& parse_args(int argc, char *argv[], io_buf* model); 
vw& parse_args(int argc, char *argv[]);
void load_input_model(vw& all, io_buf& io_temp);

LEARNER::base_learner* setup_base(vw& all);

