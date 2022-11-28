// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_to_cb_adf.h"

#include "vw/config/options.h"
#include "vw/core/cb_label_parser.h"
#include "vw/core/learner.h"
#include "vw/core/reductions/cb/cbify.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"
#include "vw/core/vw_versions.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class cb_to_cb_adf
{
public:
  parameters* weights = nullptr;
  VW::reductions::cbify_adf_data adf_data;
  bool explore_mode = false;
  multi_learner* adf_learner = nullptr;
};

template <bool is_learn>
void predict_or_learn(cb_to_cb_adf& data, multi_learner& base, VW::example& ec)
{
  data.adf_data.copy_example_to_adf(*data.weights, ec);

  CB::label backup_ld;
  CB::label new_ld;
  bool is_test_label = CB::is_test_label(ec.l.cb);

  uint32_t chosen_action = 0;
  uint32_t index_with_cost = 0;

  if (!is_test_label)
  {
    // cb.costs can be more than one in --cb
    // we need to keep the observed cost one only
    if (ec.l.cb.costs.size() > 1)
    {
      for (auto c : ec.l.cb.costs)
      {
        if (c.has_observed_cost())
        {
          chosen_action = c.action - 1;
          break;
        }
        index_with_cost++;
      }
    }
    else
    {
      chosen_action = ec.l.cb.costs[0].action - 1;
      index_with_cost = 0;
    }

    new_ld.weight = ec.l.cb.weight;
    new_ld.costs.emplace_back(ec.l.cb.costs[index_with_cost].cost, ec.l.cb.costs[index_with_cost].action - 1,
        ec.l.cb.costs[index_with_cost].probability);
  }

  if (!is_test_label && chosen_action < data.adf_data.num_actions)
  {
    backup_ld = std::move(data.adf_data.ecs[chosen_action]->l.cb);
    data.adf_data.ecs[chosen_action]->l.cb = std::move(new_ld);
  }

  auto restore_guard = VW::scope_exit([&backup_ld, &data, &chosen_action, &is_test_label, &new_ld] {
    if (!is_test_label && chosen_action < data.adf_data.num_actions)
    {
      new_ld = std::move(data.adf_data.ecs[chosen_action]->l.cb);
      data.adf_data.ecs[chosen_action]->l.cb = std::move(backup_ld);
    }
  });

  if (!base.learn_returns_prediction || !is_learn) { base.predict(data.adf_data.ecs); }
  if (is_learn) { base.learn(data.adf_data.ecs); }

  if (data.explore_mode) { ec.pred.a_s = std::move(data.adf_data.ecs[0]->pred.a_s); }
  else
  {
    ec.pred.multiclass = data.adf_data.ecs[0]->pred.a_s[0].action + 1;
  }
}

void finish_example(VW::workspace& all, cb_to_cb_adf& c, VW::example& ec)
{
  if (c.explore_mode)
  {
    c.adf_data.ecs[0]->pred.a_s = std::move(ec.pred.a_s);
    c.adf_learner->print_example(all, c.adf_data.ecs);
  }
  else
  {
    c.adf_data.ecs[0]->pred.multiclass = std::move(ec.pred.multiclass);
    c.adf_learner->print_example(all, c.adf_data.ecs);
  }
  VW::finish_example(all, ec);
}

struct options_cb_to_cb_adf_v1
{
  bool compat_old_cb = false;
  bool force_legacy = false;
  uint32_t num_actions;
  uint32_t cbx_num_actions;
  uint32_t cbi_num_actions;
  bool eval_supplied;
  bool override_cb_explore;
  bool override_cbify;
  bool override_cb;
};

std::unique_ptr<options_cb_to_cb_adf_v1> get_cb_to_cb_adf_options_instance(
    const VW::workspace& all, VW::io::logger& logger, options_i& options)
{
  auto cb_to_cb_adf_opts = VW::make_unique<options_cb_to_cb_adf_v1>();
  option_group_definition new_options("[Reduction] Contextual Bandit: cb -> cb_adf");
  new_options
      .add(make_option("cb_to_cbadf", cb_to_cb_adf_opts->num_actions)
               .help("Flag is unused and has no effect. It should not be passed. The cb_to_cbadf reduction is "
                     "automatically enabled if cb, cb_explore or cbify are used. This flag will be removed in a future "
                     "release but not the functionality."))
      .add(make_option("cb", cb_to_cb_adf_opts->num_actions)
               .keep()
               .help("Maps cb_adf to cb. Disable with cb_force_legacy"))
      .add(make_option("cb_explore", cb_to_cb_adf_opts->cbx_num_actions)
               .keep()
               .help("Translate cb explore to cb_explore_adf. Disable with cb_force_legacy"))
      .add(make_option("cbify", cb_to_cb_adf_opts->cbi_num_actions)
               .keep()
               .help("Translate cbify to cb_adf. Disable with cb_force_legacy"))
      .add(make_option("cb_force_legacy", cb_to_cb_adf_opts->force_legacy)
               .keep()
               .help("Default to non-adf cb implementation (cb_algs)"));

  options.add_and_parse(new_options);

  if (options.was_supplied("cb_to_cbadf"))
  {
    logger.out_warn(
        "The flag --cb_to_cbadf has no effect and should not be supplied. The cb_to_cbadf reduction is automatically "
        "enabled if cb, cb_explore or cbify are used. The cb_to_cbadf reduction can be force disabled with "
        "--cb_force_legacy. This flag will be removed in a future release but not the functionality.");
  }

  if (options.was_supplied("eval")) { return nullptr; }

  // ANY model created with older version should default to --cb_force_legacy
  if (all.model_file_ver != VW::version_definitions::EMPTY_VERSION_FILE)
  {
    cb_to_cb_adf_opts->compat_old_cb = !(all.model_file_ver >= VW::version_definitions::VERSION_FILE_WITH_CB_TO_CBADF);
  }

  // not compatible with adf
  if (options.was_supplied("cbify_reg")) { cb_to_cb_adf_opts->compat_old_cb = true; }

  if (cb_to_cb_adf_opts->force_legacy) { cb_to_cb_adf_opts->compat_old_cb = true; }

  cb_to_cb_adf_opts->override_cb = options.was_supplied("cb");
  cb_to_cb_adf_opts->override_cb_explore = options.was_supplied("cb_explore");
  cb_to_cb_adf_opts->override_cbify = options.was_supplied("cbify");

  if (!cb_to_cb_adf_opts->override_cb && !cb_to_cb_adf_opts->override_cb_explore && !cb_to_cb_adf_opts->override_cbify)
  {
    // do nothing
    return nullptr;
  }
  else
  {
    if (cb_to_cb_adf_opts->compat_old_cb)
    {
      options.insert("cb_force_legacy", "");
      // make sure cb_force_legacy gets serialized to the model on write
      options.add_and_parse(new_options);
      return nullptr;
    }
  }

  // if cb_explore_adf is being specified this is a noop
  if (cb_to_cb_adf_opts->override_cbify && options.was_supplied("cb_explore_adf")) { return nullptr; }

  if (cb_to_cb_adf_opts->override_cbify) { options.insert("cb_explore_adf", ""); }

  // user specified "cb_explore" but we're not using an old model file
  if (cb_to_cb_adf_opts->override_cb_explore)
  {
    cb_to_cb_adf_opts->num_actions = cb_to_cb_adf_opts->cbx_num_actions;
    options.insert("cb_explore_adf", "");
  }
  else  // if (override_cb) case
  {
    // force cb_adf; cb_adf will pick up cb_type
    options.insert("cb_adf", "");
  }

  if (cb_to_cb_adf_opts->num_actions <= 0 && !cb_to_cb_adf_opts->override_cbify)
  { THROW("cb num actions must be positive"); }

  return cb_to_cb_adf_opts;
}

}  // namespace

/*
    Purpose: run before cb, cb_explore, cbify and cb_adf related reductions
    This will 'translate' cb (non adf) input commands into their cb_adf counterparts
    Except when:
        - the model file loaded is from a version older than and not including 8.11.0
        - user bypasses this translation step using '--cb_force_legacy'

    Related files: cb_algs.cc, cb_explore.cc, cbify.cc
*/
VW::LEARNER::base_learner* VW::reductions::cb_to_cb_adf_setup(VW::setup_base_i& stack_builder)
{
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto cb_to_cb_adf_opts = get_cb_to_cb_adf_options_instance(all, all.logger, *stack_builder.get_options());
  if (cb_to_cb_adf_opts == nullptr) { return nullptr; }

  if (cb_to_cb_adf_opts->override_cbify)
  {
    // no need to register custom predict/learn, cbify will take care of that
    return stack_builder.setup_base_learner();
  }

  auto data = VW::make_unique<cb_to_cb_adf>();
  data->explore_mode = cb_to_cb_adf_opts->override_cb_explore;
  data->weights = &(all.weights);

  multi_learner* base = as_multiline(stack_builder.setup_base_learner());

  data->adf_data.init_adf_data(
      cb_to_cb_adf_opts->num_actions, base->increment, all.interactions, all.extent_interactions);

  // see csoaa.cc ~ line 894 / setup for csldf_setup
  all.example_parser->emptylines_separate_examples = false;
  VW::prediction_type_t in_pred_type;
  VW::prediction_type_t out_pred_type;

  if (data->explore_mode)
  {
    data->adf_learner = as_multiline(base->get_learner_by_name_prefix("cb_explore_adf_"));
    in_pred_type = VW::prediction_type_t::ACTION_PROBS;
    out_pred_type = VW::prediction_type_t::ACTION_PROBS;
  }
  else
  {
    data->adf_learner = as_multiline(base->get_learner_by_name_prefix("cb_adf"));
    in_pred_type = VW::prediction_type_t::ACTION_SCORES;
    out_pred_type = VW::prediction_type_t::MULTICLASS;
  }

  auto* l = make_reduction_learner(
      std::move(data), base, predict_or_learn<true>, predict_or_learn<false>, all.get_setupfn_name(cb_to_cb_adf_setup))
                .set_input_label_type(VW::label_type_t::CB)
                .set_output_label_type(VW::label_type_t::CB)
                .set_input_prediction_type(in_pred_type)
                .set_output_prediction_type(out_pred_type)
                .set_learn_returns_prediction(true)
                .set_finish_example(::finish_example)
                .build(&all.logger);

  return make_base(*l);
}
