// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/classweight.h"

#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"

#include <unordered_map>

using namespace VW::config;

namespace
{
class classweights
{
public:
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

      if (!klass.size() || !weight.size()) { THROW("error while parsing --classweight " << item); }

      int klass_int = std::stoi(klass);
      float weight_double = std::stof(weight);

      weights[klass_int] = weight_double;
    }
  }

  float get_class_weight(uint32_t klass)
  {
    auto got = weights.find(klass);
    if (got == weights.end()) { return 1.0f; }
    else { return got->second; }
  }
};

template <VW::prediction_type_t pred_type>
void update_example_weight(classweights& cweights, VW::example& ec)
{
  switch (pred_type)
  {
    case VW::prediction_type_t::SCALAR:
      ec.weight *= cweights.get_class_weight(static_cast<uint32_t>(ec.l.simple.label));
      break;
    case VW::prediction_type_t::MULTICLASS:
      ec.weight *= cweights.get_class_weight(ec.l.multi.label);
      break;
    default:
      // suppress the warning
      break;
  }
}

template <bool is_learn, VW::prediction_type_t pred_type>
void predict_or_learn(classweights& cweights, VW::LEARNER::learner& base, VW::example& ec)
{
  if (is_learn)
  {
    update_example_weight<pred_type>(cweights, ec);
    base.learn(ec);
  }
  else { base.predict(ec); }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::classweight_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  std::vector<std::string> classweight_array;
  auto cweights = VW::make_unique<classweights>();
  option_group_definition new_options("[Reduction]  Importance Weight Classes");
  new_options.add(
      make_option("classweight", classweight_array).necessary().help("Importance weight multiplier for class"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  for (auto& s : classweight_array) { cweights->load_string(s); }
  all.logger.err_info("parsed {} class weights", cweights->weights.size());

  auto base = require_singleline(stack_builder.setup_base_learner());

  std::string name_addition;
  void (*learn_ptr)(classweights&, VW::LEARNER::learner&, VW::example&);
  void (*pred_ptr)(classweights&, VW::LEARNER::learner&, VW::example&);
  VW::prediction_type_t pred_type;

  if (base->get_output_prediction_type() == VW::prediction_type_t::SCALAR)
  {
    name_addition = "-scalar";
    learn_ptr = predict_or_learn<true, VW::prediction_type_t::SCALAR>;
    pred_ptr = predict_or_learn<false, VW::prediction_type_t::SCALAR>;
    pred_type = VW::prediction_type_t::SCALAR;
  }
  else if (base->get_output_prediction_type() == VW::prediction_type_t::MULTICLASS)
  {
    name_addition = "-multi";
    learn_ptr = predict_or_learn<true, VW::prediction_type_t::MULTICLASS>;
    pred_ptr = predict_or_learn<false, VW::prediction_type_t::MULTICLASS>;
    pred_type = VW::prediction_type_t::MULTICLASS;
  }
  else { THROW("--classweight not implemented for this type of prediction"); }

  auto l = make_reduction_learner(
      std::move(cweights), base, learn_ptr, pred_ptr, stack_builder.get_setupfn_name(classweight_setup) + name_addition)
               .set_output_prediction_type(pred_type)
               .build();

  return l;
}
