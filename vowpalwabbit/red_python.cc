// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "red_python.h"

#include "reductions.h"
#include "vw.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace RED_PYTHON
{
void learn(ExternalBinding& external_binding, single_learner& base, example& ec)
{
  external_binding.SetBaseLearner(&base);
  external_binding.ActualLearn(&ec);
}

void predict(ExternalBinding& external_binding, single_learner& base, example& ec)
{
  external_binding.SetBaseLearner(&base);
  external_binding.ActualPredict(&ec);
}

void multi_learn(ExternalBinding& external_binding, multi_learner& base, multi_ex& examples)
{
  external_binding.SetBaseLearner(&base);
  external_binding.ActualLearn(&examples);
}

void multi_predict(ExternalBinding& external_binding, multi_learner& base, multi_ex& examples)
{
  external_binding.SetBaseLearner(&base);
  external_binding.ActualPredict(&examples);
}

void finish_example(vw& all, ExternalBinding& external_binding, example& ec)
{
  external_binding.ActualFinishExample(&ec);
  VW::finish_example(all, ec);
}

void finish_multiex(vw& all, ExternalBinding& external_binding, multi_ex& examples)
{
  external_binding.ActualFinishExample(&examples);
  VW::finish_example(all, examples);
}

void save_load(ExternalBinding& external_binding, io_buf& model_file, bool read, bool text)
{
  external_binding.ActualSaveLoad(&model_file, read, text);
}

}  // namespace RED_PYTHON
using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup(VW::setup_base_i& stack_builder, std::unique_ptr<RED_PYTHON::ExternalBinding> instance)
{
  if (instance == nullptr) { return nullptr; };

  vw& all = *stack_builder.get_all_pointer();

  auto base = as_singleline(stack_builder.setup_base_learner());

  VW::LEARNER::learner<ExternalBinding, example>& ret =
      learner<ExternalBinding, example>::init_learner(instance.get(), base, learn, predict, 1, base->pred_type,
          all.get_setupfn_name(red_python_setup), base->learn_returns_prediction);

  if (instance->ShouldRegisterFinishExample()) ret.set_finish_example(finish_example);

  if (instance->ShouldRegisterSaveLoad()) ret.set_save_load(save_load);

  // learner should delete ext_binding
  instance.release();

  return make_base(ret);
}

using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup(VW::setup_base_i& stack_builder)
{
  vw& all = *stack_builder.get_all_pointer();

  // force a temp throw for now
  if (!all.ext_binding) { throw("stop sailing!"); return nullptr; };

  return red_python_setup(stack_builder, std::move(all.ext_binding));
}

using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_multiline_setup(VW::setup_base_i& stack_builder)
{
  vw& all = *stack_builder.get_all_pointer();
  if (!all.ext_binding) return nullptr;

  auto base = as_multiline(stack_builder.setup_base_learner());

  VW::LEARNER::learner<ExternalBinding, multi_ex>& ret = learner<ExternalBinding, multi_ex>::init_learner(
      all.ext_binding.get(), base, multi_learn, multi_predict, 1, prediction_type_t::action_probs,
      all.get_setupfn_name(red_python_multiline_setup), base->learn_returns_prediction);

  if (all.ext_binding->ShouldRegisterFinishExample()) ret.set_finish_example(finish_multiex);

  if (all.ext_binding->ShouldRegisterSaveLoad()) ret.set_save_load(save_load);

  // learner should delete ext_binding
  all.ext_binding.release();

  return make_base(ret);
}

using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_base_setup(VW::setup_base_i& stack_builder)
{
  vw& all = *stack_builder.get_all_pointer();
  if (!all.ext_binding) return nullptr;

  learner<ExternalBinding, example>& l = init_learner_py(all.ext_binding.get(), learn, predict, 1,
      prediction_type_t::scalar, all.get_setupfn_name(red_python_base_setup), true);

  if (all.ext_binding->ShouldRegisterFinishExample()) l.set_finish_example(finish_example);

  if (all.ext_binding->ShouldRegisterSaveLoad()) l.set_save_load(save_load);

  all.ext_binding.release();

  return make_base(l);
}