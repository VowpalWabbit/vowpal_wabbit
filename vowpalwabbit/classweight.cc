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

      if (!klass.size() || !weight.size())
      {
        THROW("error: while parsing --classweight " << item);
      }

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

template <bool is_learn, prediction_type_t pred_type>
static void predict_or_learn(classweights& cweights, LEARNER::single_learner& base, example& ec)
{
  switch (pred_type)
  {
    case prediction_type_t::scalar:
      ec.weight *= cweights.get_class_weight((uint32_t)ec.l.simple.label);
      break;
    case prediction_type_t::multiclass:
      ec.weight *= cweights.get_class_weight(ec.l.multi.label);
      break;
    default:
      // suppress the warning
      break;
  }

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);
}
}  // namespace CLASSWEIGHTS

using namespace CLASSWEIGHTS;

LEARNER::base_learner* classweight_setup(options_i& options, vw& all)
{
  std::vector<std::string> classweight_array;
  auto cweights = scoped_calloc_or_throw<classweights>();
  option_group_definition new_options("importance weight classes");
  new_options.add(make_option("classweight", classweight_array).help("importance weight multiplier for class"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("classweight"))
    return nullptr;

  for (auto& s : classweight_array) cweights->load_string(s);

  if (!all.quiet)
    all.trace_message << "parsed " << cweights->weights.size() << " class weights" << std::endl;

  LEARNER::single_learner* base = as_singleline(setup_base(options, all));

  LEARNER::learner<classweights, example>* ret;
  if (base->pred_type == prediction_type_t::scalar)
    ret = &LEARNER::init_learner<classweights>(cweights, base, predict_or_learn<true, prediction_type_t::scalar>,
        predict_or_learn<false, prediction_type_t::scalar>);
  else if (base->pred_type == prediction_type_t::multiclass)
    ret = &LEARNER::init_learner<classweights>(cweights, base, predict_or_learn<true, prediction_type_t::multiclass>,
        predict_or_learn<false, prediction_type_t::multiclass>);
  else
    THROW("--classweight not implemented for this type of prediction");
  return make_base(*ret);
}
