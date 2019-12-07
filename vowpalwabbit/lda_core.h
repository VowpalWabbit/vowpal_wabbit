/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#include "v_array.h"
#include <tuple>

LEARNER::base_learner* lda_setup(VW::config::options_i&, vw&);

void get_top_weights(vw* all, int top_words_count, int topic, std::vector<feature>& output);
