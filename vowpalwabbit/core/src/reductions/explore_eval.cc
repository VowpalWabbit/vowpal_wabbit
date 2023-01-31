// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/explore_eval.h"

#include "vw/common/random.h"
#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <cfloat>
#include <memory>

// Do evaluation of nonstationary policies.
// input = contextual bandit label
// output = chosen ranking

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class rate_target
{
  /**
   * This class is used to adjust a multiplier to be used when deciding whether to update on a given example or not.
   * The class has a target rate and uses the threshold at each step to keep an estimate of the current sampling rate,
   * and return the multiplier that is needed in order to either over sample or under sample if the current sampling
   * rate is under or over the target rate respectively
   */
  float _target_rate;
  float _sum_p;
  size_t _t;
  float _latest_rate = 1.f;

  float predict() const
  {
    if (_sum_p > 0.f) { return _target_rate * (float)_t / _sum_p; }
    else { return 1.f; }
  }

  void learn(float p)
  {
    _sum_p += p;
    _t++;
  }

public:
  rate_target() : _target_rate(1), _sum_p(0), _t(0) {}

  void set_target_rate(float rate) { _target_rate = rate; }

  float get_target_rate() const { return _target_rate; }

  float get_latest_rate() const { return _latest_rate; }

  float get_rate_and_update(float threshold)
  {
    float z = predict();
    learn(threshold);

    _latest_rate = z;
    return z;
  }
};

class explore_eval
{
public:
  VW::cb_class known_cost;
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> random_state;
  uint64_t offset = 0;
  VW::cb_label action_label;
  VW::cb_label empty_label;
  size_t example_counter = 0;
  rate_target rt_target;

  size_t update_count = 0;
  float weighted_update_count = 0;
  size_t violations = 0;
  float multiplier = 0.f;

  bool fixed_multiplier = false;
  bool target_rate_on = false;
};

void finish(explore_eval& data)
{
  if (!data.all->quiet)
  {
    *(data.all->trace_message) << "weighted update count = " << data.weighted_update_count << std::endl;
    *(data.all->trace_message) << "average accepted example weight = "
                               << data.weighted_update_count / static_cast<float>(data.update_count) << std::endl;
    if (data.violations > 0) { *(data.all->trace_message) << "violation count = " << data.violations << std::endl; }
    if (!data.fixed_multiplier) { *(data.all->trace_message) << "final multiplier = " << data.multiplier << std::endl; }
    if (data.target_rate_on)
    {
      *(data.all->trace_message) << "targeted update count = "
                                 << (data.example_counter * data.rt_target.get_target_rate()) << std::endl;
      *(data.all->trace_message) << "final rate = " << (data.rt_target.get_latest_rate()) << std::endl;
    }
  }
}

// Semantics: Currently we compute the IPS loss no matter what flags
// are specified. We print the first action and probability, based on
// ordering by scores in the final output.
void update_stats_explore_eval(const VW::workspace& all, VW::shared_data& sd, const explore_eval& data,
    const VW::multi_ex& ec_seq, VW::io::logger& /* logger */)
{
  if (ec_seq.empty()) { return; }
  const auto& ec = **(ec_seq.begin());
  if (example_is_newline_not_header_cb(ec)) { return; }

  size_t num_features = 0;

  float loss = 0.;
  VW::action_scores preds = ec.pred.a_s;
  VW::label_type_t label_type = all.example_parser->lbl_parser.label_type;

  for (size_t i = 0; i < ec_seq.size(); i++)
  {
    if (!VW::LEARNER::ec_is_example_header(*ec_seq[i], label_type)) { num_features += ec_seq[i]->get_num_features(); }
  }

  bool labeled_example = true;
  if (data.known_cost.probability > 0)
  {
    for (uint32_t i = 0; i < preds.size(); i++)
    {
      float l = get_cost_estimate(data.known_cost, preds[i].action);
      loss += l * preds[i].score;
    }
  }
  else { labeled_example = false; }

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq.size(); i++) { holdout_example &= ec_seq[i]->test_only; }

  sd.update(holdout_example, labeled_example, loss, ec.weight, num_features);
}

void print_update_explore_eval(VW::workspace& all, VW::shared_data& /*sd*/, const explore_eval& data,
    const VW::multi_ex& ec_seq, VW::io::logger& /*logger*/)
{
  if (ec_seq.empty()) { return; }
  const auto& ec = **(ec_seq.begin());
  if (example_is_newline_not_header_cb(ec)) { return; }

  bool labeled_example = true;
  if (data.known_cost.probability <= 0) { labeled_example = false; }

  VW::details::print_update_cb(all, !labeled_example, ec, &ec_seq, true, nullptr);
}

void output_example_prediction_explore_eval(
    VW::workspace& all, const explore_eval& /*data*/, const VW::multi_ex& ec_seq, VW::io::logger& /*logger*/)
{
  if (ec_seq.empty()) { return; }
  const auto& ec = **(ec_seq.begin());
  if (example_is_newline_not_header_cb(ec)) { return; }

  for (auto& sink : all.final_prediction_sink)
  {
    VW::details::print_action_score(sink.get(), ec.pred.a_s, ec.tag, all.logger);
  }

  if (all.raw_prediction != nullptr)
  {
    std::string output_string;
    std::stringstream output_string_stream(output_string);
    const auto& costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0) { output_string_stream << ' '; }
      output_string_stream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, all.logger);
    all.print_text_by_ref(all.raw_prediction.get(), "", ec_seq[0]->tag, all.logger);
  }

  VW::details::global_print_newline(all.final_prediction_sink, all.logger);
}

template <bool is_learn>
void do_actual_learning(explore_eval& data, learner& base, VW::multi_ex& ec_seq)
{
  VW::example* label_example = VW::test_cb_adf_sequence(ec_seq);

  if (label_example != nullptr)  // extract label
  {
    data.action_label = std::move(label_example->l.cb);
    label_example->l.cb = std::move(data.empty_label);
  }

  multiline_learn_or_predict<false>(base, ec_seq, data.offset);

  if (label_example != nullptr)  // restore label
  {
    label_example->l.cb = std::move(data.action_label);
    data.empty_label.costs.clear();
    data.empty_label.weight = 1.f;
  }

  data.known_cost = VW::get_observed_cost_or_default_cb_adf(ec_seq);
  if (label_example != nullptr && is_learn)
  {
    data.example_counter++;

    VW::action_scores& a_s = ec_seq[0]->pred.a_s;

    float action_probability = 0;
    bool action_found = false;
    for (size_t i = 0; i < a_s.size(); i++)
    {
      if (data.known_cost.action == a_s[i].action)
      {
        action_probability = a_s[i].score;
        action_found = true;
      }
    }

    float threshold = action_probability / data.known_cost.probability;

    if (!data.fixed_multiplier && !data.target_rate_on)
    {
      if (action_found) { data.multiplier = std::min(data.multiplier, 1.f / threshold); }
    }

    threshold *= data.multiplier;

    // rate multiplier called before we potentially return if action not found since we want to update the rate
    // prediction
    float rate_multiplier = data.rt_target.get_rate_and_update(threshold);

    if (!action_found) { return; }

    if (threshold > 1. + 1e-6) { data.violations++; }

    if (data.target_rate_on) { threshold *= rate_multiplier; }

    if (data.random_state->get_and_update_random() < threshold)
    {
      VW::example* ec_found = nullptr;
      for (VW::example*& ec : ec_seq)
      {
        if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX && ec->l.cb.costs[0].probability > 0)
        {
          ec_found = ec;
        }
        if (threshold > 1.f) { ec->weight *= threshold; }
      }

      ec_found->l.cb.costs[0].probability = action_probability;

      // weighted update count since weight can be *= threshold
      data.weighted_update_count += ec_found->weight;
      data.update_count++;

      multiline_learn_or_predict<true>(base, ec_seq, data.offset);

      // restore logged example
      if (threshold > 1.f)
      {
        float inv_threshold = 1.f / threshold;
        for (auto& ec : ec_seq) { ec->weight *= inv_threshold; }
      }

      ec_found->l.cb.costs[0].probability = data.known_cost.probability;
    }
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::explore_eval_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<explore_eval>();
  bool explore_eval_option = false;
  float target_rate = 1.f;
  option_group_definition new_options("[Reduction] Explore Evaluation");
  new_options
      .add(make_option("explore_eval", explore_eval_option)
               .keep()
               .necessary()
               .help("Evaluate explore_eval adf policies"))
      .add(make_option("multiplier", data->multiplier)
               .help("Multiplier used to make all rejection sample probabilities <= 1"))
      .add(make_option("target_rate", target_rate)
               .help("The target rate will be used to adjust the rejection rate in order to achieve an update count of "
                     "#examples * target_rate"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  data->all = &all;
  data->random_state = all.get_random_state();
  data->rt_target.set_target_rate(target_rate);
  if (options.was_supplied("target_rate")) { data->target_rate_on = true; }

  if (options.was_supplied("multiplier")) { data->fixed_multiplier = true; }
  else { data->multiplier = 1; }

  if (!options.was_supplied("cb_explore_adf")) { options.insert("cb_explore_adf", ""); }

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::cb_label_parser_global;

  auto l = make_reduction_learner(std::move(data), base, do_actual_learning<true>, do_actual_learning<false>,
      stack_builder.get_setupfn_name(explore_eval_setup))
               .set_learn_returns_prediction(true)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_update_stats(update_stats_explore_eval)
               .set_output_example_prediction(output_example_prediction_explore_eval)
               .set_print_update(print_update_explore_eval)
               .set_finish(::finish)
               .build();

  return l;
}
