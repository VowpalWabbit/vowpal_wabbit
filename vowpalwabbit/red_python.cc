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
struct red_python
{
 private:

 public:
  ExternalBinding* ext_bind_impl = nullptr;

  red_python(ExternalBinding* ext_bind_impl)
      : ext_bind_impl(ext_bind_impl)
  {
  }

  ~red_python()
  {
  }
};

void learn(red_python& redpy, single_learner& base, example& ec)
{ 
  redpy.ext_bind_impl->SetLearner(&base);
  redpy.ext_bind_impl->ActualLearn(&ec);
}

void predict(red_python& c, single_learner& base, example& ec) { return; }

}  // namespace RED_PYTHON
using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup(options_i& options, vw& all)
{
  if (!all.ext_binding)
    return nullptr;

  auto ld = scoped_calloc_or_throw<red_python>(all.ext_binding.get());

  ld->ext_bind_impl->SetRandomNumber(4);

  VW::LEARNER::learner<red_python, example>& ret =
      VW::LEARNER::init_learner(ld, as_singleline(setup_base(options, all)), learn, predict);

  //missing finish

  return make_base(ret);
}
