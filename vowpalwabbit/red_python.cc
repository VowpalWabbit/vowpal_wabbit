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
struct wrapper
{
  std::unique_ptr<RED_PYTHON::ExternalBinding> instance;
};

void base_learn(wrapper& wrap, base_learner&, example& ec)
{
  // wrap.instance->SetBaseLearner(nullptr);
  wrap.instance->ActualLearn(&ec);
}

void base_predict(wrapper& wrap, base_learner&, example& ec)
{
  // wrap.instance->SetBaseLearner(&base);
  wrap.instance->ActualPredict(&ec);
}

void learn(wrapper& wrap, single_learner& base, example& ec)
{
  wrap.instance->SetBaseLearner(&base);
  wrap.instance->ActualLearn(&ec);
}

void predict(wrapper& wrap, single_learner& base, example& ec)
{
  wrap.instance->SetBaseLearner(&base);
  wrap.instance->ActualPredict(&ec);
}

void multi_learn(wrapper& wrap, multi_learner& base, multi_ex& examples)
{
  wrap.instance->SetBaseLearner(&base);
  wrap.instance->ActualLearn(&examples);
}

void multi_predict(wrapper& wrap, multi_learner& base, multi_ex& examples)
{
  wrap.instance->SetBaseLearner(&base);
  wrap.instance->ActualPredict(&examples);
}

void finish_example(vw& all, wrapper& wrap, example& ec)
{
  wrap.instance->ActualFinishExample(&ec);
  VW::finish_example(all, ec);
}

void finish_multiex(vw& all, wrapper& wrap, multi_ex& examples)
{
  wrap.instance->ActualFinishExample(&examples);
  VW::finish_example(all, examples);
}

void save_load(wrapper& wrap, io_buf& model_file, bool read, bool text)
{
  wrap.instance->ActualSaveLoad(&model_file, read, text);
}

}  // namespace RED_PYTHON
using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup(
    VW::setup_base_i& stack_builder, std::unique_ptr<RED_PYTHON::ExternalBinding> instance)
{
  if (instance == nullptr) { return nullptr; };

  bool should_register_finish_example = instance->ShouldRegisterFinishExample();
  bool should_set_save_load = instance->ShouldRegisterSaveLoad();

  auto pr = VW::make_unique<wrapper>();
  pr->instance = std::move(instance);

  auto* l = VW::LEARNER::make_reduction_learner(std::move(pr), as_singleline(stack_builder.setup_base_learner()),
      RED_PYTHON::learn, RED_PYTHON::predict, std::string("blah"))
                .build();

  if (should_register_finish_example) l->set_finish_example(finish_example);
  if (should_set_save_load) l->set_save_load(save_load);

  pr.release();

  // return make_base(ret);
  return make_base(*l);
}

using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup_normie(VW::setup_base_i& stack_builder)
{
  vw& all = *stack_builder.get_all_pointer();

  // force a temp throw for now
  if (!all.ext_binding)
  {
    throw("stop sailing!");
    return nullptr;
  };

  return red_python_setup(stack_builder, std::move(all.ext_binding));
}

using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_multiline_setup(VW::setup_base_i& stack_builder)
{
  vw& all = *stack_builder.get_all_pointer();
  if (!all.ext_binding) return nullptr;

  auto base = as_multiline(stack_builder.setup_base_learner());

  auto pr = VW::make_unique<wrapper>();

  VW::LEARNER::learner<wrapper, multi_ex>& ret = learner<wrapper, multi_ex>::init_learner(pr.get(), base, multi_learn,
      multi_predict, 1, prediction_type_t::action_probs, all.get_setupfn_name(red_python_multiline_setup),
      base->learn_returns_prediction);

  if (all.ext_binding->ShouldRegisterFinishExample()) ret.set_finish_example(finish_multiex);

  if (all.ext_binding->ShouldRegisterSaveLoad()) ret.set_save_load(save_load);

  pr->instance = std::move(all.ext_binding);
  // learner should delete ext_binding
  // all.ext_binding.release();
  pr.release();

  return make_base(ret);
}

using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_base_setup(VW::setup_base_i& stack_builder)
{
  vw& all = *stack_builder.get_all_pointer();
  if (!all.ext_binding) return nullptr;

  bool should_register_finish_example = all.ext_binding->ShouldRegisterFinishExample();
  bool should_set_save_load = all.ext_binding->ShouldRegisterSaveLoad();

  auto pr = VW::make_unique<wrapper>();
  pr->instance = std::move(all.ext_binding);

  auto* l = VW::LEARNER::make_base_learner<wrapper, example>(std::move(pr), RED_PYTHON::base_learn,
      RED_PYTHON::base_predict, std::string("blah"), prediction_type_t::scalar, label_type_t::simple)
                .build();

  if (should_register_finish_example) l->set_finish_example(finish_example);
  if (should_set_save_load) l->set_save_load(save_load);

  return make_base(*l);
}