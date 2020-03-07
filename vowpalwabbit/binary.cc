// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include "reductions.h"

using namespace VW::config;

template <bool is_learn>
void predict_or_learn(char&, LEARNER::single_learner& base, example& ec, label_data& simple_label)
{
  if (is_learn)
    base.learn_with_label(ec, simple_label);
  else
    base.predict_with_label(ec, simple_label);

  if (ec.pred.scalar > 0)
    ec.pred.scalar = 1;
  else
    ec.pred.scalar = -1;

  const auto& label = simple_label.label;
  if (label != FLT_MAX)
  {
    if (fabs(label) != 1.f)
      std::cout << "You are using label " << label << " not -1 or 1 as loss function expects!" << std::endl;
    else if (label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ec.weight;
  }
}


LEARNER::base_learner* binary_setup(options_i& options, vw& all)
{
  bool binary = false;
  option_group_definition new_options("Binary loss");
  new_options.add(make_option("binary", binary).keep().help("report loss as binary classification on -1,1"));
  options.add_and_parse(new_options);

  if (!binary)
    return nullptr;

  using predict_learn_fn = void(*)(char&, LEARNER::single_learner&, example&);
  LEARNER::learner<char, example>& ret =
      LEARNER::init_learner(as_singleline(setup_base(options, all)), static_cast<predict_learn_fn>(nullptr), static_cast<predict_learn_fn>(nullptr));
  ret.label_type = label_type_t::simple;
  ret.set_learn(predict_or_learn<true>);
  ret.set_predict(predict_or_learn<false>);
  return make_base(ret);
}
