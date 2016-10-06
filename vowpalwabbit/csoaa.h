/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
LEARNER::base_learner* csoaa_setup(vw& all);

LEARNER::base_learner* csldf_setup(vw& all);

namespace LabelDict
{
bool ec_is_example_header(example& ec);// example headers look like "0:-1" or just "shared"
}
