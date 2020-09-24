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

void update(ExternalBinding& external_binding, VW::LEARNER::single_learner&, example& ec)
{
  external_binding.ActualUpdate(&ec);
}

float sensitivity(ExternalBinding& external_binding, VW::LEARNER::base_learner&, example& ec)
{
  return external_binding.ActualSensitivity(&ec);
}

void finish(ExternalBinding& external_binding)
{
  external_binding.ActualFinish();
}

void end_pass(ExternalBinding& external_binding)
{
  external_binding.ActualEndPass();
}

void end_examples(ExternalBinding& external_binding)
{
  external_binding.ActualEndExamples();
}


}  // namespace RED_PYTHON
using RED_PYTHON::ExternalBinding;
using VW::config::options_i;

template <typename T>
void set_learner_functions(T& learner, ExternalBinding* ext_binding)
{
  if (ext_binding->ShouldRegisterFinishExample())
    learner.set_finish_example(RED_PYTHON::finish_example);

  if(ext_binding->ShouldRegisterUpdate())
    learner.set_update(RED_PYTHON::update);

  if(ext_binding->ShouldRegisterSensitivity())
    learner.set_sensitivity(RED_PYTHON::sensitivity);

  if(ext_binding->ShouldRegisterFinish())
    learner.set_finish(RED_PYTHON::finish);

  if(ext_binding->ShouldRegisterEndPass())
    learner.set_end_pass(RED_PYTHON::end_pass);

  if(ext_binding->ShouldRegisterEndExamples())
    learner.set_end_examples(RED_PYTHON::end_examples);
}

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
  set_learner_functions(ret, ext_binding);

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
  set_learner_functions(ret, ext_binding);

  return make_base(ret);
}
