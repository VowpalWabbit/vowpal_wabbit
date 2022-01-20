// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "wrapper_reduction.h"

#include "global_data.h"
#include "reductions.h"
#include "vw.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace wrapper
{
struct wrap
{
  std::unique_ptr<wrapper::external_binding> instance;
};

void base_learn(wrap& wrap, base_learner&, example& ec) { wrap.instance->actual_learn(&ec); }

void base_predict(wrap& wrap, base_learner&, example& ec) { wrap.instance->actual_predict(&ec); }

void learn(wrap& wrap, single_learner& base, example& ec)
{
  wrap.instance->set_base_learner(&base);
  wrap.instance->actual_learn(&ec);
}

void predict(wrap& wrap, single_learner& base, example& ec)
{
  wrap.instance->set_base_learner(&base);
  wrap.instance->actual_predict(&ec);
}

void multi_learn(wrap& wrap, multi_learner& base, multi_ex& examples)
{
  wrap.instance->set_base_learner(&base);
  wrap.instance->actual_learn(&examples);
}

void multi_predict(wrap& wrap, multi_learner& base, multi_ex& examples)
{
  wrap.instance->set_base_learner(&base);
  wrap.instance->actual_predict(&examples);
}

void finish_example(VW::workspace& all, wrap& wrap, example& ec)
{
  wrap.instance->actual_finish_example(&ec);
  VW::finish_example(all, ec);
}

void finish_multiex(VW::workspace& all, wrap& wrap, multi_ex& examples)
{
  wrap.instance->actual_finish_example(&examples);
  VW::finish_example(all, examples);
}

void save_load(wrap& wrap, io_buf& model_file, bool read, bool text)
{
  wrap.instance->actual_saveload(&model_file, read, text);
}

}  // namespace wrapper
using namespace wrapper;
VW::LEARNER::base_learner* wrapper_reduction_setup(
    VW::setup_base_i& stack_builder, std::unique_ptr<wrapper::external_binding> instance)
{
  if (!instance) { return nullptr; };

  auto pr = VW::make_unique<wrap>();
  pr->instance = std::move(instance);

  bool should_register_finish_example = pr->instance->should_register_finish_example();
  bool should_set_save_load = pr->instance->should_register_saveload();

  auto l = VW::LEARNER::make_reduction_learner(std::move(pr), as_singleline(stack_builder.setup_base_learner()),
      wrapper::learn, wrapper::predict, std::string("python_single"));

  if (should_register_finish_example) l.set_finish_example(finish_example);
  if (should_set_save_load) l.set_save_load(save_load);

  auto* l2 = l.build();

  return make_base(*l2);
}

using namespace wrapper;
VW::LEARNER::base_learner* wrapper_reduction_multiline_setup(
    VW::setup_base_i& stack_builder, std::unique_ptr<wrapper::external_binding> instance)
{
  if (!instance) return nullptr;

  auto pr = VW::make_unique<wrap>();
  pr->instance = std::move(instance);

  bool should_register_finish_example = pr->instance->should_register_finish_example();
  bool should_set_save_load = pr->instance->should_register_saveload();

  auto l = VW::LEARNER::make_reduction_learner(std::move(pr), as_multiline(stack_builder.setup_base_learner()),
      wrapper::multi_learn, wrapper::multi_predict, std::string("python_multi"))
               .set_input_prediction_type(prediction_type_t::action_probs);

  if (should_register_finish_example) l.set_finish_example(finish_multiex);
  if (should_set_save_load) l.set_save_load(save_load);

  auto* l2 = l.build();

  return make_base(*l2);
}

using namespace wrapper;
VW::LEARNER::base_learner* wrapper_reduction_base_setup(
    VW::setup_base_i&, std::unique_ptr<wrapper::external_binding> instance)
{
  if (!instance) return nullptr;

  auto pr = VW::make_unique<wrap>();
  pr->instance = std::move(instance);

  bool should_register_finish_example = pr->instance->should_register_finish_example();
  bool should_set_save_load = pr->instance->should_register_saveload();

  auto l = VW::LEARNER::make_base_learner<wrap, example>(std::move(pr), wrapper::base_learn, wrapper::base_predict,
      std::string("python_base"), prediction_type_t::scalar, label_type_t::simple);

  if (should_register_finish_example) l.set_finish_example(finish_example);
  if (should_set_save_load) l.set_save_load(save_load);

  auto* l2 = l.build();

  return make_base(*l2);
}
