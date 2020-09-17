// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "red_python.h"

#include "reductions.h"
#include "learner.h"
#include "vw.h"


namespace RED_PYTHON
{
void learn(ExternalBinding& external_binding, VW::LEARNER::single_learner& base, example& ec)
{ 
  external_binding.SetBaseLearner(&base);
  external_binding.ActualLearn(&ec);
}

void predict(ExternalBinding& external_binding, VW::LEARNER::single_learner& base, example& ec) {
  external_binding.SetBaseLearner(&base);
  external_binding.ActualPredict(&ec);
}

void learn_base(ExternalBinding& external_binding, VW::LEARNER::single_learner&, example& ec)
{ 
  external_binding.SetBaseLearner(nullptr);
  external_binding.ActualLearn(&ec);
}

void predict_base(ExternalBinding& external_binding, VW::LEARNER::single_learner&, example& ec) {
  external_binding.SetBaseLearner(nullptr);
  external_binding.ActualPredict(&ec);
}  
  
void finish_example(vw& all, ExternalBinding& external_binding, example& ec) {
  external_binding.ActualFinishExample(&ec);
  // have to bubble this out to python?
  VW::finish_example(all, ec);
}

}  // namespace RED_PYTHON
using RED_PYTHON::ExternalBinding;
using VW::config::options_i;

VW::LEARNER::base_learner* red_python_setup(options_i& options, vw& all, const std::string& name, ExternalBinding* ext_binding)
{
  auto base = as_singleline(setup_base(options, all));

  auto& ret = VW::LEARNER::learner<ExternalBinding, example>::init_learner(
    ext_binding,
    base,
    RED_PYTHON::learn,
    RED_PYTHON::predict,
    (size_t)1,
    base->pred_type
  );

  ret.hash_index() = name;
  if (ext_binding->ShouldRegisterFinishExample())
    ret.set_finish_example(RED_PYTHON::finish_example);

  return make_base(ret);
}

VW::LEARNER::base_learner* red_python_base_setup(options_i&, vw& all, const std::string& name, ExternalBinding* ext_binding)
{
  // TODO: hack. Dunno why, but if we don't pass a 'base' reduction pointer, we get compile errors
  VW::LEARNER::single_learner* base = nullptr;
  auto& ret = VW::LEARNER::learner<ExternalBinding, example>::init_learner(
    ext_binding,
    base,
    RED_PYTHON::learn_base,
    RED_PYTHON::predict_base,
    (size_t)1,
    prediction_type_t::scalar
  );

  ret.hash_index() = name;
  if (ext_binding->ShouldRegisterFinishExample())
    ret.set_finish_example(RED_PYTHON::finish_example);

  return make_base(ret);
}
