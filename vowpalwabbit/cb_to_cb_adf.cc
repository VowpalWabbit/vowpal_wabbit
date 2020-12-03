// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_to_cb_adf.h"

#include "reductions.h"
#include "learner.h"
#include "vw.h"
#include "cbify.h"
#include "cb_algs.h"

using namespace LEARNER;
using namespace VW::config;

struct cb_to_cb_adf
{
  parameters* weights;
  cbify_adf_data adf_data;

  ~cb_to_cb_adf()
  {
    for (size_t a = 0; a < adf_data.num_actions; ++a)
    {
      adf_data.ecs[a]->pred.a_s.delete_v();
      VW::dealloc_example(CB::cb_label.delete_label, *adf_data.ecs[a]);
      free_it(adf_data.ecs[a]);
    }
  }
};

template <bool is_learn>
void predict_or_learn(cb_to_cb_adf& data, multi_learner& base, example& ec)
{
//   CB::label ld = ec.l.cb;
  copy_example_to_adf(data.adf_data, *(data.weights), ec);

  if (is_learn)
  {
    // base.predict(data.adf_data.ecs);
    uint32_t chosen_action = ec.l.cb.costs[0].action - 1;
    if (chosen_action < data.adf_data.num_actions)
    {
      data.adf_data.ecs[chosen_action]->l.cb = ec.l.cb;
      base.learn(data.adf_data.ecs);
    }
    else
    {
      std::cerr << "warning: malformed label blah outside of range bluh. skipping." << std::endl;
      base.predict(data.adf_data.ecs);
    }
  }
  else
  {
    // Guard example state restore against throws
    // auto restore_guard = VW::scope_exit([&ld, &ec] { ec.l.cb = ld; });

    base.predict(data.adf_data.ecs);
  }

  // prediction from adf_data.ecs -> copy to ec.label -> take argmin
  // cb_adf => first action is a greedy action TODO: is this a contract?
  ec.pred.multiclass = data.adf_data.ecs[0]->pred.a_s[0].action;
//   data.adf_data.ecs[0]->pred.a_s[0].score
}

void output_example(vw& all, cb_to_cb_adf& data, example& ec, CB::label& ld)
{
  float loss = 0.;

// inline float get_cost_estimate(ACTION_SCORE::action_score& a_s, float cost, uint32_t action, float offset = 0.)
// import cb_algs.h -> ips estimate
// no bueno
  loss = CB_ALGS::get_cost_estimate(data.adf_data.ecs[0]->pred.a_s[0], ld.costs[0].cost, ld.costs[0].action);

  all.sd->update(ec.test_only, !CB::cb_label.test_label(&ld), loss, 1.f, ec.num_features);

  for (auto& sink : all.final_prediction_sink) all.print_by_ref(sink.get(), (float)ec.pred.multiclass, 0, ec.tag);

  if (all.raw_prediction != nullptr)
  {
    std::stringstream outputStringStream;
    for (unsigned int i = 0; i < ld.costs.size(); i++)
    {
      CB::cb_class cl = ld.costs[i];
      if (i > 0) outputStringStream << ' ';
      outputStringStream << cl.action << ':' << cl.partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, CB::cb_label.test_label(&ld), ec, nullptr, false);
}

void finish_example(vw& all, cb_to_cb_adf& c, example& ec)
{
  output_example(all, c, ec, ec.l.cb);
  VW::finish_example(all, ec);
}

VW::LEARNER::base_learner* cb_to_cb_adf_setup(options_i& options, vw& all)
{
  bool eval = false;
  auto data = scoped_calloc_or_throw<cb_to_cb_adf>();

  option_group_definition new_options("Contextual Bandit Options");
  new_options
      .add(make_option("new_cb", data->adf_data.num_actions).keep().necessary().help("Use contextual bandit learning with <k> costs"))
      .add(make_option("eval", eval).help("Evaluate a policy rather than optimizing."));

  if (!options.add_parse_and_check_necessary(new_options))
    return nullptr;

  // force cb_adf -> cb_adf will pick up cb_type
  options.insert("cb_adf", "");

  data->weights = &(all.weights);

  init_adf_data(data->adf_data, data->adf_data.num_actions, all.interactions);

  multi_learner* base = as_multiline(setup_base(options, all));

  learner<cb_to_cb_adf, example>* l;
  // multiclass is inferior to action_scores (as cb_adf does)
  // for compat reasons we stick to multiclass for now
  l = &init_learner(
    data, base, predict_or_learn<true>, predict_or_learn<false>, 1, prediction_type_t::multiclass);
  l->set_finish_example(finish_example);

  return make_base(*l);
}