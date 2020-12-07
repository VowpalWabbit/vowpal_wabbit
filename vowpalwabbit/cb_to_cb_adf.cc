// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_to_cb_adf.h"

#include "vw_versions.h"
#include "reductions.h"
#include "learner.h"
#include "vw.h"
#include "cbify.h"
#include "cb_algs.h"
#include "cb_label_parser.h"

using namespace VW::LEARNER;
using namespace VW::config;

struct cb_to_cb_adf
{
  parameters* weights;
  cbify_adf_data adf_data;
};

template <bool is_learn>
void predict_or_learn(cb_to_cb_adf& data, multi_learner& base, example& ec)
{
  data.adf_data.copy_example_to_adf(*data.weights, ec);

  if (is_learn && !CB::is_test_label(&ec.l.cb))
  {
    uint32_t chosen_action = ec.l.cb.costs[0].action - 1;
    if (chosen_action < data.adf_data.num_actions)
    {
      // TODO: do i need a guard here?
      CB::label ld = data.adf_data.ecs[chosen_action]->l.cb;
      data.adf_data.ecs[chosen_action]->l.cb = ec.l.cb;
      base.learn(data.adf_data.ecs);
      data.adf_data.ecs[chosen_action]->l.cb = ld;

      CB::default_label(&data.adf_data.ecs[chosen_action]->l.cb);
    }
    else
    {
      std::cerr << "warning: malformed label blah outside of range bluh. skipping." << std::endl;
      base.predict(data.adf_data.ecs);
    }
  }
  else
  {
    base.predict(data.adf_data.ecs);
  }

  // cb_adf => first action is a greedy action TODO: is this a contract?
  ec.pred.multiclass = data.adf_data.ecs[0]->pred.a_s[0].action + 1;
}

void output_example(vw& all, example& ec, CB::label& ld)
{
  float loss = CB_ALGS::get_cost_estimate(ld, ec.pred.multiclass);
  CB_ALGS::generic_output_example(all, loss, ec, ld);
}

void finish_example(vw& all, cb_to_cb_adf&, example& ec)
{
  output_example(all, ec, ec.l.cb);
  VW::finish_example(all, ec);
}

VW::LEARNER::base_learner* cb_to_cb_adf_setup(options_i& options, vw& all)
{
  bool compat_old_cb = false;
  bool force_legacy = false;
  bool eval = false;
  std::string type_string = "mtr";
  uint32_t num_actions;

  option_group_definition new_options("Contextual Bandit Options");
  new_options
      .add(make_option("cb", num_actions).keep().necessary().help("Use contextual bandit learning with <k> costs"))
      .add(make_option("cb_type", type_string).keep().help("contextual bandit method to use in {}"))
      .add(make_option("eval", eval).help("Evaluate a policy rather than optimizing."))
      .add(make_option("force_legacy", force_legacy).help("Default to old cb implementation"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  // if user specified both old_cb and cb, we default to old_cb
  if (options.was_supplied("old_cb")) return nullptr;

  // models created with older version should default to --old_cb
  if (all.model_file_ver != EMPTY_VERSION_FILE) compat_old_cb = !(all.model_file_ver >= VERSION_FILE_WITH_CB_TO_CBADF);

  // not implemented in "new_cb" yet
  if (eval || compat_old_cb || force_legacy)
  {
    options.insert("old_cb", std::to_string(num_actions));
    return nullptr;
  }

  // force cb_adf; cb_adf will pick up cb_type
  options.insert("cb_adf", "");

  auto data = scoped_calloc_or_throw<cb_to_cb_adf>();
  data->weights = &(all.weights);
  data->adf_data.init_adf_data(num_actions, all.interactions);

  multi_learner* base = as_multiline(setup_base(options, all));

  learner<cb_to_cb_adf, example>* l;
  // multiclass is inferior to action_scores (as cb_adf does)
  // for compat reasons we stick to multiclass for now
  l = &init_learner(data, base, predict_or_learn<true>, predict_or_learn<false>, 1, prediction_type_t::multiclass);
  l->set_finish_example(finish_example);

  all.delete_prediction = nullptr;

  return make_base(*l);
}
