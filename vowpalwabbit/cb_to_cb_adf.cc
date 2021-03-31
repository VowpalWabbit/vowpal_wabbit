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
#include "cb_explore.h"
#include "cb_label_parser.h"

using namespace VW::LEARNER;
using namespace VW::config;

struct cb_to_cb_adf
{
  parameters* weights;
  cbify_adf_data adf_data;
  bool explore_mode;
  bool learn_returns_prediction;
};

template <bool is_learn>
void predict_or_learn(cb_to_cb_adf& data, multi_learner& base, example& ec)
{
  data.adf_data.copy_example_to_adf(*data.weights, ec);

  if (!data.learn_returns_prediction || !is_learn) { base.predict(data.adf_data.ecs); }

  if (is_learn && !CB::is_test_label(ec.l.cb))
  {
    uint32_t chosen_action = ec.l.cb.costs[0].action - 1;
    if (chosen_action < data.adf_data.num_actions)
    {
      CB::label ld = data.adf_data.ecs[chosen_action]->l.cb;
      data.adf_data.ecs[chosen_action]->l.cb = ec.l.cb;
      auto restore_guard = VW::scope_exit([&data, &ld, chosen_action] { data.adf_data.ecs[chosen_action]->l.cb = ld; });

      base.learn(data.adf_data.ecs);
    }
  }

  if (data.explore_mode) { ec.pred.a_s = std::move(data.adf_data.ecs[0]->pred.a_s); }
  else
  {
    // cb_adf => first action is a greedy action TODO: is this a contract?
    ec.pred.multiclass = data.adf_data.ecs[0]->pred.a_s[0].action + 1;
  }
}

float calc_loss(example& ec, CB::label& ld)
{
  float loss = 0.;

  if (!CB::is_test_label(ld))
  {
    auto optional_cost = CB::get_observed_cost_cb(ld);
    // cost observed, not default
    if (optional_cost.first == true)
    {
      for (uint32_t i = 0; i < ec.pred.a_s.size(); i++)
      { loss += CB_ALGS::get_cost_estimate(optional_cost.second, ec.pred.a_s[i].action + 1) * ec.pred.a_s[i].score; }
    }
  }

  return loss;
}

void output_example(vw& all, bool explore_mode, example& ec, CB::label& ld)
{
  if (explore_mode)
  {
    float loss = calc_loss(ec, ld);
    CB_EXPLORE::generic_output_example(all, loss, ec, ld);
  }
  else
  {
    float loss = CB_ALGS::get_cost_estimate(ld, ec.pred.multiclass);
    CB_ALGS::generic_output_example(all, loss, ec, ld);
  }
}

void finish_example(vw& all, cb_to_cb_adf& c, example& ec)
{
  output_example(all, c.explore_mode, ec, ec.l.cb);

  if (c.explore_mode) c.adf_data.ecs[0]->pred.a_s = std::move(ec.pred.a_s);

  VW::finish_example(all, ec);
}

/*
    Purpose: run before cb, cb_explore, cbify and cb_adf related reductions
    This will 'translate' cb (non adf) input commands into their cb_adf counterparts
    Except when:
        - the model file loaded is from a version older or including 8.9.0
        - user bypasses this translation step using '--cb_force_legacy'
        - user specifies the cb_type to 'dm' (not implemented in adf)

    Related files: cb_algs.cc, cb_explore.cc, cbify.cc
*/
VW::LEARNER::base_learner* cb_to_cb_adf_setup(options_i& options, vw& all)
{
  bool compat_old_cb = false;
  bool force_legacy = false;
  std::string type_string = "mtr";
  uint32_t num_actions;
  uint32_t cbx_num_actions;
  uint32_t cbi_num_actions;

  option_group_definition new_options("Contextual Bandit Options");
  new_options
      .add(make_option("cb_to_cbadf", num_actions).necessary().help("Maps cb_adf to cb. Disable with cb_force_legacy."))
      .add(make_option("cb", num_actions).keep().help("Maps cb_adf to cb. Disable with cb_force_legacy."))
      .add(make_option("cb_explore", cbx_num_actions)
               .keep()
               .help("Translate cb explore to cbexploreadf. Disable with cb_force_legacy."))
      .add(make_option("cbify", cbi_num_actions)
               .keep()
               .help("Translate cbify to cbexploreadf. Disable with cb_force_legacy."))
      .add(make_option("cb_type", type_string).keep().help("contextual bandit method to use in {}"))
      .add(make_option("cb_force_legacy", force_legacy).keep().help("Default to old cb implementation"));

  options.add_parse_and_check_necessary(new_options);

  if (options.was_supplied("eval")) return nullptr;

  // ANY model created with older version should default to --cb_force_legacy
  if (all.model_file_ver != EMPTY_VERSION_FILE) compat_old_cb = !(all.model_file_ver >= VERSION_FILE_WITH_CB_TO_CBADF);

  // not compatible with adf
  if (options.was_supplied("cbify_reg")) compat_old_cb = true;

  // dm not implemented in cb_adf
  if (type_string == "dm") compat_old_cb = true;

  if (force_legacy) compat_old_cb = true;

  bool override_cb = options.was_supplied("cb");
  bool override_cb_explore = options.was_supplied("cb_explore");
  bool override_cbify = options.was_supplied("cbify");

  if (!override_cb && !override_cb_explore && !override_cbify)
  {
    // do nothing
    return nullptr;
  }
  else
  {
    if (compat_old_cb)
    {
      options.insert("cb_force_legacy", "");
      return nullptr;
    }
  }

  // if cb_explore_adf is being specified this is a noop
  if (override_cbify && options.was_supplied("cb_explore_adf")) return nullptr;

  if (override_cbify)
  {
    options.insert("cb_explore_adf", "");
    // no need to register custom predict/learn, cbify will take care of that
    return setup_base(options, all);
  }

  // user specified "cb_explore" but we're not using an old model file
  if (override_cb_explore)
  {
    num_actions = cbx_num_actions;
    options.insert("cb_explore_adf", "");
  }
  else  // if (override_cb) case
  {
    // force cb_adf; cb_adf will pick up cb_type
    options.insert("cb_adf", "");
  }

  auto data = scoped_calloc_or_throw<cb_to_cb_adf>();
  data->explore_mode = override_cb_explore;
  data->weights = &(all.weights);
  data->adf_data.init_adf_data(num_actions, all.interactions);

  multi_learner* base = as_multiline(setup_base(options, all));

  learner<cb_to_cb_adf, example>* l;

  data->learn_returns_prediction = base->learn_returns_prediction;
  if (data->explore_mode)
  {
    l = &init_learner(data, base, predict_or_learn<true>, predict_or_learn<false>, 1, prediction_type_t::action_probs,
        "cb_to_cb_adf", base->learn_returns_prediction);
  }
  else
  {
    l = &init_learner(data, base, predict_or_learn<true>, predict_or_learn<false>, 1, prediction_type_t::multiclass,
        "cb_to_cb_adf", base->learn_returns_prediction);
  }

  l->set_finish_example(finish_example);

  return make_base(*l);
}
