// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <unordered_map>
#include "reductions.h"

using namespace VW::config;

namespace CLASSWEIGHTS
{
struct classweights
{
  std::unordered_map<uint32_t, float> weights;

  void load_string(std::string const& source)
  {
    std::stringstream ss(source);
    std::string item;
    while (std::getline(ss, item, ','))
    {
      std::stringstream inner_ss(item);
      std::string klass;
      std::string weight;
      std::getline(inner_ss, klass, ':');
      std::getline(inner_ss, weight, ':');

      if (!klass.size() || !weight.size()) { THROW("error: while parsing --classweight " << item); }

      int klass_int = std::stoi(klass);
      float weight_double = std::stof(weight);

      weights[klass_int] = weight_double;
    }
  }

  float get_class_weight(uint32_t klass)
  {
    auto got = weights.find(klass);
    if (got == weights.end())
      return 1.0f;
    else
      return got->second;
  }
};

template <VW::prediction_type_t pred_type>
void update_example_weight(classweights& cweights, example& ec)
{
  switch (pred_type)
  {
    case VW::prediction_type_t::scalar:
      ec.weight *= cweights.get_class_weight(static_cast<uint32_t>(ec.l.simple.label));
      break;
    case VW::prediction_type_t::multiclass:
      ec.weight *= cweights.get_class_weight(ec.l.multi.label);
      break;
    default:
      // suppress the warning
      break;
  }
}

template <bool is_learn, VW::prediction_type_t pred_type>
void predict_or_learn(classweights& cweights, VW::LEARNER::single_learner& base, example& ec)
{
  if (is_learn)
  {
    update_example_weight<pred_type>(cweights, ec);
    base.learn(ec);
  }
  else
  {
    base.predict(ec);
  }
}
}  // namespace CLASSWEIGHTS

using namespace CLASSWEIGHTS;

VW::LEARNER::base_learner* classweight_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();
  std::vector<std::string> classweight_array;
  auto cweights = VW::make_unique<classweights>();
  option_group_definition new_options("importance weight classes");
  new_options.add(
      make_option("classweight", classweight_array).necessary().help("importance weight multiplier for class"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  for (auto& s : classweight_array) cweights->load_string(s);

  if (!all.logger.quiet) *(all.trace_message) << "parsed " << cweights->weights.size() << " class weights" << std::endl;

  VW::LEARNER::single_learner* base = as_singleline(stack_builder.setup_base_learner());

  std::string name_addition;
  void (*learn_ptr)(classweights&, VW::LEARNER::single_learner&, example&);
  void (*pred_ptr)(classweights&, VW::LEARNER::single_learner&, example&);
  VW::prediction_type_t pred_type;

  if (base->get_output_prediction_type() == VW::prediction_type_t::scalar)
  {
    name_addition = "-scalar";
    learn_ptr = predict_or_learn<true, prediction_type_t::scalar>;
    pred_ptr = predict_or_learn<false, prediction_type_t::scalar>;
    pred_type = VW::prediction_type_t::scalar;
  }
  else if (base->get_output_prediction_type() == prediction_type_t::multiclass)
  {
    name_addition = "-multi";
    learn_ptr = predict_or_learn<true, prediction_type_t::multiclass>;
    pred_ptr = predict_or_learn<false, prediction_type_t::multiclass>;
    pred_type = VW::prediction_type_t::multiclass;
  }
  else
  {
    THROW("--classweight not implemented for this type of prediction");
  }

  auto* l = make_reduction_learner(
      std::move(cweights), base, learn_ptr, pred_ptr, stack_builder.get_setupfn_name(classweight_setup) + name_addition)
                .set_prediction_type(pred_type)
                .build();

  return make_base(*l);
}
