// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "wrapper_reduction.h"

#include "reductions.h"
#include "vw.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace WRAPPER
{
struct wrapper
{
  std::unique_ptr<WRAPPER::ExternalBinding> instance;
};

void base_learn(wrapper& wrap, base_learner&, example& ec) { wrap.instance->ActualLearn(&ec); }

void base_predict(wrapper& wrap, base_learner&, example& ec) { wrap.instance->ActualPredict(&ec); }

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

}  // namespace WRAPPER
using namespace WRAPPER;
VW::LEARNER::base_learner* wrapper_reduction_setup(
    VW::setup_base_i& stack_builder, std::unique_ptr<WRAPPER::ExternalBinding> instance)
{
  if (!instance) { return nullptr; };

  auto pr = VW::make_unique<wrapper>();
  pr->instance = std::move(instance);

  bool should_register_finish_example = pr->instance->ShouldRegisterFinishExample();
  bool should_set_save_load = pr->instance->ShouldRegisterSaveLoad();

  auto* l = VW::LEARNER::make_reduction_learner(std::move(pr), as_singleline(stack_builder.setup_base_learner()),
      WRAPPER::learn, WRAPPER::predict, std::string("python_single"))
                .build();

  if (should_register_finish_example) l->set_finish_example(finish_example);
  if (should_set_save_load) l->set_save_load(save_load);

  return make_base(*l);
}

using namespace WRAPPER;
VW::LEARNER::base_learner* wrapper_reduction_multiline_setup(
    VW::setup_base_i& stack_builder, std::unique_ptr<WRAPPER::ExternalBinding> instance)
{
  if (!instance) return nullptr;

  auto base = as_multiline(stack_builder.setup_base_learner());

  auto pr = VW::make_unique<wrapper>();
  pr->instance = std::move(instance);

  VW::LEARNER::learner<wrapper, multi_ex>& ret = learner<wrapper, multi_ex>::init_learner(pr.get(), base, multi_learn,
      multi_predict, 1, prediction_type_t::action_probs, std::string("python_multi"), base->learn_returns_prediction);

  if (pr->instance->ShouldRegisterFinishExample()) ret.set_finish_example(finish_multiex);
  if (pr->instance->ShouldRegisterSaveLoad()) ret.set_save_load(save_load);

  pr.release();

  return make_base(ret);
}

using namespace WRAPPER;
VW::LEARNER::base_learner* wrapper_reduction_base_setup(
    VW::setup_base_i&, std::unique_ptr<WRAPPER::ExternalBinding> instance)
{
  if (!instance) return nullptr;

  auto pr = VW::make_unique<wrapper>();
  pr->instance = std::move(instance);

  bool should_register_finish_example = pr->instance->ShouldRegisterFinishExample();
  bool should_set_save_load = pr->instance->ShouldRegisterSaveLoad();

  auto* l = VW::LEARNER::make_base_learner<wrapper, example>(std::move(pr), WRAPPER::base_learn, WRAPPER::base_predict,
      std::string("python_base"), prediction_type_t::scalar, label_type_t::simple)
                .build();

  if (should_register_finish_example) l->set_finish_example(finish_example);
  if (should_set_save_load) l->set_save_load(save_load);

  return make_base(*l);
}
