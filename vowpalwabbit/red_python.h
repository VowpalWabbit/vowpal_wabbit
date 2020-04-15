// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <vector>
#include "reductions_fwd.h"

VW::LEARNER::base_learner* red_python_setup(VW::config::options_i& options, vw& all);

namespace RED_PYTHON
{
    struct Copperhead
    {
        int random_number = 0;
        void* run_object;  // for python this will really be a (py::object*), but we don't want basic VW to have to know about
        void (*run_f)(RED_PYTHON::Copperhead&);
    };
}  // namespace RED_PYTHON
