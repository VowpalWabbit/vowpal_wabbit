/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
// This is a learner which does nothing with examples.  Used when VW is used as a compressor.

#include "reductions.h"

void learn(char&, LEARNER::base_learner&, example&) {}

LEARNER::base_learner* noop_setup(arguments& arg)
{
  if (arg.new_options("Noop Learner").critical("noop", "do no learning").missing()) return nullptr;

  return &LEARNER::init_learner<char>(nullptr, learn, 1);
}
