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

template <int pred_type>
static inline void update_example_weight(classweights& cweights, example& ec)
{
  switch (pred_type)
  {
    case prediction_type::scalar:
      ec.weight *= cweights.get_class_weight((uint32_t)ec.l.simple.label);
      break;
    case prediction_type::multiclass:
      ec.weight *= cweights.get_class_weight(ec.l.multi.label);
      break;
    default:
      // suppress the warning
      break;
  }
}

template <bool is_learn, int pred_type>
static void predict_or_learn(classweights& cweights, LEARNER::single_learner& base, example& ec)
{
  // Notes: Update example weight either in predict() or learn() but not in both
  // If base.predict_before_learn is set, predict() will be called before learn()

  if (is_learn){
    if(!base.predict_before_learn)
      update_example_weight<pred_type>(cweights, ec);
    base.learn(ec);
  }
  else{
    update_example_weight<pred_type>(cweights, ec);
    base.predict(ec);
  }
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
  if (base->pred_type == prediction_type::scalar)
    ret = &LEARNER::init_learner<classweights>(cweights, base, predict_or_learn<true, prediction_type::scalar>,
        predict_or_learn<false, prediction_type::scalar>, "classweight-scalar", base->predict_before_learn);
  else if (base->pred_type == prediction_type::multiclass)
    ret = &LEARNER::init_learner<classweights>(cweights, base, predict_or_learn<true, prediction_type::multiclass>,
        predict_or_learn<false, prediction_type::multiclass>,"classweight-multi", base->predict_before_learn);
  else
    THROW("--classweight not implemented for this type of prediction");
  return make_base(*ret);
}
