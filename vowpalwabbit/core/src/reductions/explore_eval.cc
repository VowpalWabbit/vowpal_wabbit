// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/explore_eval.h"

#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/print_utils.h"
#include "vw/core/rand_state.h"
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
using namespace CB_ALGS;
using namespace VW::config;

namespace
{
class explore_eval
{
public:
  CB::cb_class known_cost;
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> random_state;
  uint64_t offset = 0;
  CB::label action_label;
  CB::label empty_label;
  size_t example_counter = 0;
  size_t block_size = 1;
  size_t block_ex_quota = 1;

  size_t update_count = 0;
  size_t violations = 0;
  float multiplier = 0.f;

  bool fixed_multiplier = false;
};

void finish(explore_eval& data)
{
  if (!data.all->quiet)
  {
    *(data.all->trace_message) << "update count = " << data.update_count << std::endl;
    if (data.violations > 0) { *(data.all->trace_message) << "violation count = " << data.violations << std::endl; }
    if (!data.fixed_multiplier) { *(data.all->trace_message) << "final multiplier = " << data.multiplier << std::endl; }
    if (data.block_size > 1)
    {
      *(data.all->trace_message) << "targeted update count = " << (data.example_counter / data.block_size) << std::endl;
    }
  }
}

// Semantics: Currently we compute the IPS loss no matter what flags
// are specified. We print the first action and probability, based on
// ordering by scores in the final output.

void output_example(VW::workspace& all, const explore_eval& c, const VW::example& ec, const VW::multi_ex* ec_seq)
{
  if (example_is_newline_not_header(ec)) { return; }

  size_t num_features = 0;

  float loss = 0.;
  VW::action_scores preds = (*ec_seq)[0]->pred.a_s;
  VW::label_type_t label_type = all.example_parser->lbl_parser.label_type;

  for (size_t i = 0; i < (*ec_seq).size(); i++)
  {
    if (!VW::LEARNER::ec_is_example_header(*(*ec_seq)[i], label_type))
    { num_features += (*ec_seq)[i]->get_num_features(); }
  }

  bool labeled_example = true;
  if (c.known_cost.probability > 0)
  {
    for (uint32_t i = 0; i < preds.size(); i++)
    {
      float l = get_cost_estimate(c.known_cost, preds[i].action);
      loss += l * preds[i].score;
    }
  }
  else
  {
    labeled_example = false;
  }

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq->size(); i++) { holdout_example &= (*ec_seq)[i]->test_only; }

  all.sd->update(holdout_example, labeled_example, loss, ec.weight, num_features);

  for (auto& sink : all.final_prediction_sink)
  { VW::details::print_action_score(sink.get(), ec.pred.a_s, ec.tag, all.logger); }

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
  }

  CB::print_update(all, !labeled_example, ec, ec_seq, true, nullptr);
}

void output_example_seq(VW::workspace& all, const explore_eval& data, const VW::multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example(all, data, **(ec_seq.begin()), &(ec_seq));
    if (all.raw_prediction != nullptr)
    { all.print_text_by_ref(all.raw_prediction.get(), "", ec_seq[0]->tag, all.logger); }
  }
}

void finish_multiline_example(VW::workspace& all, explore_eval& data, VW::multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example_seq(all, data, ec_seq);
    VW::details::global_print_newline(all.final_prediction_sink, all.logger);
  }
  VW::finish_example(all, ec_seq);
}

template <bool is_learn>
void do_actual_learning(explore_eval& data, multi_learner& base, VW::multi_ex& ec_seq)
{
  VW::example* label_example = CB_ADF::test_adf_sequence(ec_seq);

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

  data.known_cost = CB_ADF::get_observed_cost_or_default_cb_adf(ec_seq);
  if (label_example != nullptr && is_learn)
  {
    data.example_counter++;

    if (data.example_counter % data.block_size == 0)
    {
      // once new block has rolled over either reset quota or roll over previous quota
      data.block_ex_quota++;
    }
    else if (data.block_ex_quota == 0)
    {
      // if we reached the example quota for this example block just skip
      return;
    }

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

    if (!action_found) { return; }

    float threshold = action_probability / data.known_cost.probability;

    if (!data.fixed_multiplier) { data.multiplier = std::min(data.multiplier, 1 / threshold); }

    threshold *= data.multiplier;

    if (threshold > 1. + 1e-6) { data.violations++; }

    if (data.random_state->get_and_update_random() < threshold)
    {
      VW::example* ec_found = nullptr;
      for (VW::example*& ec : ec_seq)
      {
        if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX && ec->l.cb.costs[0].probability > 0)
        { ec_found = ec; }
        if (threshold > 1) { ec->weight *= threshold; }
      }

      ec_found->l.cb.costs[0].probability = action_probability;

      multiline_learn_or_predict<true>(base, ec_seq, data.offset);

      // restore logged example
      if (threshold > 1)
      {
        float inv_threshold = 1.f / threshold;
        for (auto& ec : ec_seq) { ec->weight *= inv_threshold; }
      }

      ec_found->l.cb.costs[0].probability = data.known_cost.probability;

      data.update_count++;
      data.block_ex_quota--;
    }
  }
}

struct options_explore_eval_v1
{
  bool explore_eval_option = false;
  uint32_t block_size = 0;
  float multiplier;
  bool multiplier_supplied;
};

std::unique_ptr<options_explore_eval_v1> get_explore_eval_options_instance(
    const VW::workspace&, VW::io::logger&, options_i& options)
{
  auto explore_eval_opts = VW::make_unique<options_explore_eval_v1>();
  option_group_definition new_options("[Reduction] Explore Evaluation");
  new_options
      .add(make_option("explore_eval", explore_eval_opts->explore_eval_option)
               .keep()
               .necessary()
               .help("Evaluate explore_eval adf policies"))
      .add(make_option("multiplier", explore_eval_opts->multiplier)
               .help("Multiplier used to make all rejection sample probabilities <= 1"))
      .add(
          make_option("block_size", explore_eval_opts->block_size)
              .default_value(1)
              .help("The examples will be processed in blocks of block_size. If an example update is found in that "
                    "block no other examples in the block will be used to update the policy. If an example is not used "
                    "in the block then the quota rolls over and the next block can update more than one examples"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  if (!options.was_supplied("cb_explore_adf")) { options.insert("cb_explore_adf", ""); }
  explore_eval_opts->multiplier_supplied = options.was_supplied("multiplier");
  return explore_eval_opts;
}

}  // namespace

base_learner* VW::reductions::explore_eval_setup(VW::setup_base_i& stack_builder)
{
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto explore_eval_opts = get_explore_eval_options_instance(all, all.logger, *stack_builder.get_options());
  if (explore_eval_opts == nullptr) { return nullptr; }

  auto explore_eval_data = VW::make_unique<explore_eval>();

  explore_eval_data->multiplier = explore_eval_opts->multiplier;
  explore_eval_data->all = &all;
  explore_eval_data->random_state = all.get_random_state();
  explore_eval_data->block_size = static_cast<size_t>(explore_eval_opts->block_size);
  explore_eval_data->block_ex_quota = 1;

  if (explore_eval_opts->multiplier_supplied) { explore_eval_data->fixed_multiplier = true; }
  else
  {
    explore_eval_data->multiplier = 1;
  }

  multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  auto* l = make_reduction_learner(std::move(explore_eval_data), base, do_actual_learning<true>, do_actual_learning<false>,
      stack_builder.get_setupfn_name(explore_eval_setup))
                .set_learn_returns_prediction(true)
                .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
                .set_input_label_type(VW::label_type_t::CB)
                .set_finish_example(finish_multiline_example)
                .set_finish(::finish)
                .build();

  return make_base(*l);
}
