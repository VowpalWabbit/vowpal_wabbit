// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "red_python.h"

#include "reductions.h"
#include "vw.h"

using namespace LEARNER;
using namespace VW::config;

namespace RED_PYTHON
{
//useful for debugging
void learn(ExternalBinding& redpy, single_learner& base, example& ec)
{ 
  redpy.SetLearner(&base);
  redpy.ActualLearn(&ec);
}

//useful for debugging
void predict(ExternalBinding& c, single_learner& base, example& ec) { return; }

}  // namespace RED_PYTHON
using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup(options_i& options, vw& all)
{
  if (!all.ext_binding)
    return nullptr;

  all.ext_binding->SetRandomNumber(4);

  auto base = as_singleline(setup_base(options, all));

  VW::LEARNER::learner<ExternalBinding, example>& ret =
      learner<ExternalBinding, example>::init_learner(all.ext_binding.get(), base, learn, predict, 1, base->pred_type);

  // learner should delete ext_binding
  all.ext_binding.release();

  //missing finish?

  return make_base(ret);
}
