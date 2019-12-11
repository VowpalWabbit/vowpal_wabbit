// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This is a learner which does nothing with examples.  Used when VW is used as a compressor.

#include "reductions.h"

using namespace VW::config;

void learn(char&, LEARNER::base_learner&, example&) {}

LEARNER::base_learner* noop_setup(options_i& options, vw&)
{
  bool noop = false;
  option_group_definition new_options("Noop Learner");
  new_options.add(make_option("noop", noop).keep().help("do no learning"));
  options.add_and_parse(new_options);

  if (!noop)
    return nullptr;

  return make_base(LEARNER::init_learner(learn, 1));
}
