// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_to_cb_adf.h"

#include "reductions.h"
#include "learner.h"
#include "vw.h"

using namespace LEARNER;
using namespace VW::config;

//predict learn cbify


VW::LEARNER::base_learner* cb_to_cb_adf_setup(options_i& options, vw& all)
{
  option_group_definition new_options("Contextual Bandit Options");
  new_options
      .add(make_option("cb", data->cbcs.num_actions).keep().necessary().help("Use contextual bandit learning with <k> costs"))
      .add(make_option("eval", eval).help("Evaluate a policy rather than optimizing."));

  if (!options.add_parse_and_check_necessary(new_options))
    return nullptr;

  //add here cb_adf argument -> cb_adf will pick up cb_type
  options.insert("cb_adf");

  multi_learner* base = as_multiline(setup_base(options, all));

  // update interactions on setup
  // make sure interactions are parsed at this point, most probable
}