// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_adf.h"

#include "vw/common/string_view.h"
#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/label_parser.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/explore/explore.h"

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::CB_ADF

#include "vw/io/logger.h"

using namespace VW::LEARNER;
using namespace VW::config;

VW::cb_class VW::get_observed_cost_or_default_cb_adf(const VW::multi_ex& examples)
{
  bool found = false;
  uint32_t found_index = 0;
  uint32_t i = 0;
  VW::cb_class known_cost;

  for (const auto* example_ptr : examples)
  {
    for (const auto& cost : example_ptr->l.cb.costs)
    {
      if (cost.has_observed_cost())
      {
        found = true;
        found_index = i;
        known_cost = cost;
      }
    }
    i++;
  }

  if (found == false)
  {
    known_cost.probability = -1;
    return known_cost;
  }

  known_cost.action = found_index;
  return known_cost;
}
// Validates a multiline example collection as a valid sequence for action dependent features format.
VW::example* VW::test_cb_adf_sequence(const VW::multi_ex& ec_seq)
{
  if (ec_seq.empty()) THROW("cb_adf: At least one action must be provided for an example to be valid.");

  uint32_t count = 0;
  VW::example* ret = nullptr;
  for (auto* ec : ec_seq)
  {
    // Check if there is more than one cost for this example.
    if (ec->l.cb.costs.size() > 1)
    {
      auto message = fmt::format(
          "cb_adf: badly formatted example, only one cost can be known but found {}. Example number={}, tag={}",
          ec->l.cb.costs.size(), ec->example_counter, VW::string_view{ec->tag.data(), ec->tag.size()});
      THROW(message);
    }

    // Check whether the cost was initialized to a value.
    if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX)
    {
      ret = ec;
      count += 1;
      if (count > 1) THROW("cb_adf: badly formatted example, only one line can have a cost");
    }
  }

  return ret;
}

const VW::version_struct* VW::reductions::cb_adf::get_model_file_ver() const { return &_all->model_file_ver; }

void VW::reductions::cb_adf::learn_ips(learner& base, VW::multi_ex& examples)
{
  details::gen_cs_example_ips(examples, _cs_labels, _all->logger, _clip_p);
  details::cs_ldf_learn_or_predict<true>(base, examples, _cb_labels, _cs_labels, _prepped_cs_labels, true, _offset);
}

void VW::reductions::cb_adf::learn_sm(learner& base, VW::multi_ex& examples)
{
  details::gen_cs_test_example(examples, _cs_labels);  // create test labels.
  details::cs_ldf_learn_or_predict<false>(base, examples, _cb_labels, _cs_labels, _prepped_cs_labels, false, _offset);

  // Can probably do this more efficiently than 6 loops over the examples...
  //[1: initialize temporary storage;
  // 2: find chosen action;
  // 3: create cs_labels (gen_cs_example_sm);
  // 4: get probability of chosen action;
  // 5: backup example wts;
  // 6: restore example wts]
  _a_s.clear();
  _prob_s.clear();
  // TODO: Check that predicted scores are always stored with the first example
  for (uint32_t i = 0; i < examples[0]->pred.a_s.size(); i++)
  {
    _a_s.push_back({examples[0]->pred.a_s[i].action, examples[0]->pred.a_s[i].score});
    _prob_s.push_back({examples[0]->pred.a_s[i].action, 0.0});
  }

  float sign_offset = 1.0;  // To account for negative rewards/costs
  uint32_t chosen_action = 0;
  float example_weight = 1.0;

  for (uint32_t i = 0; i < examples.size(); i++)
  {
    VW::cb_label ld = examples[i]->l.cb;
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
    {
      chosen_action = i;
      example_weight = ld.costs[0].cost / details::safe_probability(ld.costs[0].probability, _all->logger);

      // Importance weights of examples cannot be negative.
      // So we use a trick: set |w| as weight, and use sign(w) as an offset in the regression target.
      if (ld.costs[0].cost < 0.0)
      {
        sign_offset = -1.0;
        example_weight = -example_weight;
      }
      break;
    }
  }

  details::gen_cs_example_sm(examples, chosen_action, sign_offset, _a_s, _cs_labels);

  // Lambda is -1 in the call to generate_softmax because in vw, lower score is better; for softmax higher score is
  // better.
  VW::explore::generate_softmax(-1.0, begin_scores(_a_s), end_scores(_a_s), begin_scores(_prob_s), end_scores(_prob_s));

  // TODO: Check Marco's example that causes VW to report prob > 1.

  for (auto const& action_score : _prob_s)  // Scale example_wt by prob of chosen action
  {
    if (action_score.action == chosen_action)
    {
      example_weight *= action_score.score;
      break;
    }
  }

  _backup_weights.clear();
  _backup_nf.clear();
  for (auto const& action_score : _prob_s)
  {
    uint32_t current_action = action_score.action;
    _backup_weights.push_back(examples[current_action]->weight);
    _backup_nf.push_back(static_cast<uint32_t>(examples[current_action]->num_features));

    if (current_action == chosen_action)
    {
      examples[current_action]->weight *= example_weight * (1.0f - action_score.score);
    }
    else { examples[current_action]->weight *= example_weight * action_score.score; }

    if (examples[current_action]->weight <= 1e-15) { examples[current_action]->weight = 0; }
  }

  // Do actual training
  details::cs_ldf_learn_or_predict<true>(base, examples, _cb_labels, _cs_labels, _prepped_cs_labels, false, _offset);

  // Restore example weights and numFeatures
  for (size_t i = 0; i < _prob_s.size(); i++)
  {
    uint32_t current_action = _prob_s[i].action;
    examples[current_action]->weight = _backup_weights[i];
    examples[current_action]->num_features = _backup_nf[i];
  }
}

void VW::reductions::cb_adf::learn_dr(learner& base, VW::multi_ex& examples)
{
  details::gen_cs_example_dr<true>(gen_cs, examples, _cs_labels, _clip_p);
  details::cs_ldf_learn_or_predict<true>(base, examples, _cb_labels, _cs_labels, _prepped_cs_labels, true, _offset);
}

void VW::reductions::cb_adf::learn_dm(learner& base, VW::multi_ex& examples)
{
  details::gen_cs_example_dm(examples, _cs_labels);
  details::cs_ldf_learn_or_predict<true>(base, examples, _cb_labels, _cs_labels, _prepped_cs_labels, true, _offset);
}

template <bool PREDICT>
void VW::reductions::cb_adf::learn_mtr(learner& base, VW::multi_ex& examples)
{
  if (PREDICT)  // first get the prediction to return
  {
    details::gen_cs_example_ips(examples, _cs_labels, _all->logger);
    details::cs_ldf_learn_or_predict<false>(base, examples, _cb_labels, _cs_labels, _prepped_cs_labels, false, _offset);
    std::swap(examples[0]->pred.a_s, _a_s);
  }

  // Train on _one_ action (which requires up to 3 examples).
  // We must go through the cost sensitive classifier layer to get
  // proper feature handling.
  gen_cs_example_mtr(gen_cs, examples, _cs_labels);
  uint32_t nf = static_cast<uint32_t>(examples[gen_cs.mtr_example]->num_features);
  float old_weight = examples[gen_cs.mtr_example]->weight;
  const float clipped_p = std::max(examples[gen_cs.mtr_example]->l.cb.costs[0].probability, _clip_p);
  examples[gen_cs.mtr_example]->weight *=
      1.f / clipped_p * (static_cast<float>(gen_cs.event_sum) / static_cast<float>(gen_cs.action_sum));

  std::swap(gen_cs.mtr_ec_seq[0]->pred.a_s, _a_s_mtr_cs);
  // TODO!!! cb_labels are not getting properly restored (empty costs are
  // dropped)
  details::cs_ldf_learn_or_predict<true>(
      base, gen_cs.mtr_ec_seq, _cb_labels, _cs_labels, _prepped_cs_labels, false, _offset);
  examples[gen_cs.mtr_example]->num_features = nf;
  examples[gen_cs.mtr_example]->weight = old_weight;
  std::swap(gen_cs.mtr_ec_seq[0]->pred.a_s, _a_s_mtr_cs);

  if (PREDICT)
  {  // Return the saved prediction
    std::swap(examples[0]->pred.a_s, _a_s);
  }
}

void VW::reductions::cb_adf::learn(learner& base, VW::multi_ex& ec_seq)
{
  if (VW::test_cb_adf_sequence(ec_seq) != nullptr)
  {
    _offset = ec_seq[0]->ft_offset;
    gen_cs.known_cost = VW::get_observed_cost_or_default_cb_adf(ec_seq);  // need to set for test case
    switch (gen_cs.cb_type)
    {
      case VW::cb_type_t::DR:
        learn_dr(base, ec_seq);
        break;
      case VW::cb_type_t::DM:
        learn_dm(base, ec_seq);
        break;
      case VW::cb_type_t::IPS:
        learn_ips(base, ec_seq);
        break;
      case VW::cb_type_t::MTR:
        if (_no_predict) { learn_mtr<false>(base, ec_seq); }
        else { learn_mtr<true>(base, ec_seq); }
        break;
      case VW::cb_type_t::SM:
        learn_sm(base, ec_seq);
        break;
    }
  }
  else if (learn_returns_prediction()) { predict(base, ec_seq); }
}

void VW::reductions::cb_adf::predict(learner& base, VW::multi_ex& ec_seq)
{
  _offset = ec_seq[0]->ft_offset;
  gen_cs.known_cost = VW::get_observed_cost_or_default_cb_adf(ec_seq);  // need to set for test case
  details::gen_cs_test_example(ec_seq, _cs_labels);                     // create test labels.
  details::cs_ldf_learn_or_predict<false>(base, ec_seq, _cb_labels, _cs_labels, _prepped_cs_labels, false, _offset);
}

// how to

bool VW::reductions::cb_adf::update_statistics(
    const VW::example& ec, const VW::multi_ex& ec_seq, VW::shared_data& sd) const
{
  size_t num_features = 0;

  uint32_t action = ec.pred.a_s[0].action;
  for (const auto& example : ec_seq) { num_features += example->get_num_features(); }

  float loss = 0.;

  bool labeled_example = true;
  if (gen_cs.known_cost.probability > 0) { loss = get_cost_estimate(gen_cs.known_cost, gen_cs.pred_scores, action); }
  else { labeled_example = false; }

  bool holdout_example = labeled_example;
  for (auto const& i : ec_seq) { holdout_example &= i->test_only; }

  sd.update(holdout_example, labeled_example, loss, ec.weight, num_features);
  return labeled_example;
}

namespace
{

void update_stats_cb_adf(const VW::workspace& /* all */, VW::shared_data& sd, const VW::reductions::cb_adf& data,
    const VW::multi_ex& ec_seq, VW::io::logger& /* logger */)
{
  if (ec_seq.empty()) { return; }
  const auto& ec = *ec_seq.front();
  data.update_statistics(ec, ec_seq, sd);
}

void output_example_prediction_cb_adf(
    VW::workspace& all, const VW::reductions::cb_adf& data, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  if (ec_seq.empty()) { return; }
  const auto& ec = *ec_seq.front();
  for (auto& sink : all.final_prediction_sink)
  {
    if (data.get_rank_all()) { VW::details::print_action_score(sink.get(), ec.pred.a_s, ec.tag, logger); }
    else
    {
      const uint32_t action = ec.pred.a_s[0].action;
      all.print_by_ref(sink.get(), static_cast<float>(action), 0, ec.tag, logger);
    }
  }
  VW::details::global_print_newline(all.final_prediction_sink, logger);
}

void print_update_cb_adf(VW::workspace& all, VW::shared_data& /* sd */, const VW::reductions::cb_adf& data,
    const VW::multi_ex& ec_seq, VW::io::logger& /* unused */)
{
  if (ec_seq.empty()) { return; }

  const bool labeled_example = data.gen_cs.known_cost.probability > 0;
  const auto& ec = *ec_seq.front();
  if (labeled_example) { VW::details::print_update_cb(all, !labeled_example, ec, &ec_seq, true, data.known_cost()); }
  else { VW::details::print_update_cb(all, !labeled_example, ec, &ec_seq, true, nullptr); }
}

void save_load(VW::reductions::cb_adf& c, VW::io_buf& model_file, bool read, bool text)
{
  if (c.get_model_file_ver() != nullptr &&
      *c.get_model_file_ver() < VW::version_definitions::VERSION_FILE_WITH_CB_ADF_SAVE)
  {
    return;
  }

  std::stringstream msg;
  msg << "event_sum " << c.get_gen_cs().event_sum << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, (char*)&c.get_gen_cs().event_sum, sizeof(c.get_gen_cs().event_sum), read, msg, text);

  msg << "action_sum " << c.get_gen_cs().action_sum << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, (char*)&c.get_gen_cs().action_sum, sizeof(c.get_gen_cs().action_sum), read, msg, text);
}

void cb_adf_merge(const std::vector<float>& /* per_model_weights */,
    const std::vector<const VW::reductions::cb_adf*>& sources, VW::reductions::cb_adf& output_data)
{
  for (const auto* source : sources)
  {
    output_data.get_gen_cs().event_sum += source->get_gen_cs().event_sum;
    output_data.get_gen_cs().action_sum += source->get_gen_cs().action_sum;
  }
}

void cb_adf_add(
    const VW::reductions::cb_adf& data1, const VW::reductions::cb_adf& data2, VW::reductions::cb_adf& data_out)
{
  data_out.get_gen_cs().event_sum = data1.get_gen_cs().event_sum + data2.get_gen_cs().event_sum;
  data_out.get_gen_cs().action_sum = data1.get_gen_cs().action_sum + data2.get_gen_cs().action_sum;
}

void cb_adf_subtract(
    const VW::reductions::cb_adf& data1, const VW::reductions::cb_adf& data2, VW::reductions::cb_adf& data_out)
{
  data_out.get_gen_cs().event_sum = data1.get_gen_cs().event_sum - data2.get_gen_cs().event_sum;
  data_out.get_gen_cs().action_sum = data1.get_gen_cs().action_sum - data2.get_gen_cs().action_sum;
}

void learn(VW::reductions::cb_adf& c, learner& base, VW::multi_ex& ec_seq) { c.learn(base, ec_seq); }

void predict(VW::reductions::cb_adf& c, learner& base, VW::multi_ex& ec_seq) { c.predict(base, ec_seq); }

}  // namespace
std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_adf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  bool cb_adf_option = false;
  std::string type_string = "mtr";

  VW::cb_type_t cb_type;
  bool rank_all;
  float clip_p;
  bool no_predict = false;

  option_group_definition new_options("[Reduction] Contextual Bandit with Action Dependent Features");
  new_options
      .add(make_option("cb_adf", cb_adf_option)
               .keep()
               .necessary()
               .help("Do Contextual Bandit learning with multiline action dependent features"))
      .add(make_option("rank_all", rank_all).keep().help("Return actions sorted by score order"))
      .add(make_option("no_predict", no_predict).help("Do not do a prediction when training"))
      .add(make_option("clip_p", clip_p)
               .keep()
               .default_value(0.f)
               .help("Clipping probability in importance weight. Default: 0.f (no clipping)"))
      .add(make_option("cb_type", type_string)
               .keep()
               .default_value("mtr")
               .one_of({"ips", "dm", "dr", "mtr", "sm"})
               .help("Contextual bandit method to use"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Ensure serialization of this option in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  if (options.was_supplied("indexing"))
  {
    THROW("--indexing is not compatible with contextual bandits, please remove this option")
  }

  // number of weight vectors needed
  size_t problem_multiplier = 1;  // default for IPS
  bool check_baseline_enabled = false;

  try
  {
    cb_type = VW::cb_type_from_string(type_string);
  }
  catch (const VW::vw_exception& /*exception*/)
  {
    all.logger.err_warn(
        "cb_type must be in {{'ips','dr','mtr','dm','sm'}}; resetting to mtr. Input was: '{}'", type_string);
    cb_type = VW::cb_type_t::MTR;
  }

  if (cb_type == VW::cb_type_t::DR)
  {
    problem_multiplier = 2;
    // only use baseline when manually enabled for loss estimation
    check_baseline_enabled = true;
  }

  if (clip_p > 0.f && cb_type == VW::cb_type_t::SM)
  {
    all.logger.err_warn("Clipping probability not yet implemented for cb_type sm; p will not be clipped.");
  }

  // Push necessary flags.
  if ((!options.was_supplied("csoaa_ldf") && !options.was_supplied("wap_ldf")) || rank_all ||
      !options.was_supplied("csoaa_rank"))
  {
    if (!options.was_supplied("csoaa_ldf")) { options.insert("csoaa_ldf", "multiline"); }

    if (!options.was_supplied("csoaa_rank")) { options.insert("csoaa_rank", ""); }
  }

  if (options.was_supplied("baseline") && check_baseline_enabled) { options.insert("check_enabled", ""); }

  auto ld = VW::make_unique<VW::reductions::cb_adf>(cb_type, rank_all, clip_p, no_predict, &all);

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::cb_label_parser_global;

  VW::reductions::cb_adf* bare = ld.get();
  bool lrp = ld->learn_returns_prediction();
  auto l = make_reduction_learner(std::move(ld), base, learn, predict, stack_builder.get_setupfn_name(cb_adf_setup))
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CS)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_learn_returns_prediction(lrp)
               .set_params_per_weight(problem_multiplier)
               .set_save_load(::save_load)
               .set_merge(::cb_adf_merge)
               .set_add(::cb_adf_add)
               .set_subtract(::cb_adf_subtract)
               .set_output_example_prediction(::output_example_prediction_cb_adf)
               .set_print_update(::print_update_cb_adf)
               .set_update_stats(::update_stats_cb_adf)
               .build();

  bare->set_scorer(VW::LEARNER::require_singleline(base->get_learner_by_name_prefix("scorer")));

  return l;
}
