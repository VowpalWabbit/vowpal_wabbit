// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_algs.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <sstream>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class cb
{
public:
  VW::details::cb_to_cs cbcs;
  VW::io::logger logger;

  cb(VW::io::logger logger) : logger(std::move(logger)) {}
};

template <bool is_learn>
void predict_or_learn(cb& data, learner& base, VW::example& ec)
{
  VW::details::cb_to_cs& c = data.cbcs;
  auto optional_cost = get_observed_cost_cb(ec.l.cb);
  // cost observed, not default
  if (optional_cost.first) { c.known_cost = optional_cost.second; }
  else { c.known_cost = VW::cb_class{}; }

  // cost observed, not default
  if (optional_cost.first && (c.known_cost.action < 1 || c.known_cost.action > c.num_actions))
  {
    data.logger.err_error("invalid action: {}", c.known_cost.action);
  }

  // generate a cost-sensitive example to update classifiers
  VW::details::gen_cs_example<is_learn>(c, ec, ec.l.cb, ec.l.cs, data.logger);

  if (c.cb_type != VW::cb_type_t::DM)
  {
    if (is_learn) { base.learn(ec); }
    else { base.predict(ec); }

    for (size_t i = 0; i < ec.l.cb.costs.size(); i++)
    {
      ec.l.cb.costs[i].partial_prediction = ec.l.cs.costs[i].partial_prediction;
    }
  }
}

void predict_eval(cb&, learner&, VW::example&) { THROW("can not use a test label for evaluation"); }

void learn_eval(cb& data, learner&, VW::example& ec)
{
  VW::details::cb_to_cs& c = data.cbcs;
  auto optional_cost = get_observed_cost_cb(ec.l.cb_eval.event);
  // cost observed, not default
  if (optional_cost.first) { c.known_cost = optional_cost.second; }
  else { c.known_cost = VW::cb_class{}; }
  VW::details::gen_cs_example<true>(c, ec, ec.l.cb_eval.event, ec.l.cs, data.logger);

  for (size_t i = 0; i < ec.l.cb_eval.event.costs.size(); i++)
  {
    ec.l.cb_eval.event.costs[i].partial_prediction = ec.l.cs.costs[i].partial_prediction;
  }

  ec.pred.multiclass = ec.l.cb_eval.action;
}

template <bool uses_eval>
void update_stats_cb_algs(const VW::workspace& /* all */, VW::shared_data& sd, const cb& data, const VW::example& ec,
    VW::io::logger& /* unused */)
{
  const auto& ld = uses_eval ? ec.l.cb_eval.event : ec.l.cb;
  const auto& c = data.cbcs;
  float loss = 0.;

  if (!ld.is_test_label()) { loss = VW::get_cost_estimate(c.known_cost, c.pred_scores, ec.pred.multiclass); }

  sd.update(ec.test_only, !ld.is_test_label(), loss, 1.f, ec.get_num_features());
}

template <bool uses_eval>
void output_example_prediction_cb_algs(
    VW::workspace& all, const cb& /* data */, const VW::example& ec, VW::io::logger& logger)
{
  const auto& ld = uses_eval ? ec.l.cb_eval.event : ec.l.cb;

  for (auto& sink : all.final_prediction_sink)
  {
    all.print_by_ref(sink.get(), static_cast<float>(ec.pred.multiclass), 0, ec.tag, all.logger);
  }

  if (all.raw_prediction != nullptr)
  {
    std::stringstream output_string_stream;
    for (unsigned int i = 0; i < ld.costs.size(); i++)
    {
      VW::cb_class cl = ld.costs[i];
      if (i > 0) { output_string_stream << ' '; }
      output_string_stream << cl.action << ':' << cl.partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, logger);
  }
}

template <bool uses_eval>
void print_update_cb_algs(
    VW::workspace& all, VW::shared_data& /* sd */, const cb& data, const VW::example& ec, VW::io::logger& /* unused */)
{
  const auto& ld = uses_eval ? ec.l.cb_eval.event : ec.l.cb;
  const auto& c = data.cbcs;

  bool is_ld_test_label = ld.is_test_label();
  if (!is_ld_test_label) { VW::details::print_update_cb(all, is_ld_test_label, ec, nullptr, false, &c.known_cost); }
  else { VW::details::print_update_cb(all, is_ld_test_label, ec, nullptr, false, nullptr); }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_algs_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<cb>(all.logger);
  std::string type_string = "dr";
  bool eval = false;
  bool force_legacy = true;

  option_group_definition new_options("[Reduction] Contextual Bandit");
  new_options
      .add(make_option("cb", data->cbcs.num_actions)
               .keep()
               .necessary()
               .help("Use contextual bandit learning with <k> costs"))
      .add(make_option("cb_type", type_string)
               .keep()
               .default_value("dr")
               .one_of({"ips", "dm", "dr", "mtr", "sm"})
               .help("Contextual bandit method to use"))
      .add(make_option("eval", eval).help("Evaluate a policy rather than optimizing"))
      .add(make_option("cb_force_legacy", force_legacy)
               .keep()
               .help("Default to non-adf cb implementation (cb_to_cb_adf)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (!eval && !force_legacy) { return nullptr; }

  // Ensure serialization of this option in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  VW::details::cb_to_cs& c = data->cbcs;

  size_t problem_multiplier = 2;  // default for DR
  c.cb_type = VW::cb_type_from_string(type_string);
  switch (c.cb_type)
  {
    case VW::cb_type_t::DR:
      break;
    case VW::cb_type_t::DM:
      if (eval) THROW("direct method can not be used for evaluation --- it is biased.");
      problem_multiplier = 1;
      break;
    case VW::cb_type_t::IPS:
      problem_multiplier = 1;
      break;
    case VW::cb_type_t::MTR:
    case VW::cb_type_t::SM:
      data->logger.err_warn(
          "cb_type must be in {{'ips','dm','dr'}}; resetting to dr. Input received: {}", VW::to_string(c.cb_type));
      c.cb_type = VW::cb_type_t::DR;
      break;
  }

  if (!options.was_supplied("csoaa"))
  {
    std::stringstream ss;
    ss << data->cbcs.num_actions;
    options.insert("csoaa", ss.str());
  }

  auto base = require_singleline(stack_builder.setup_base_learner());
  if (eval) { all.example_parser->lbl_parser = VW::cb_eval_label_parser_global; }
  else { all.example_parser->lbl_parser = VW::cb_label_parser_global; }
  c.scorer = VW::LEARNER::require_singleline(base->get_learner_by_name_prefix("scorer"));

  std::string name_addition = eval ? "-eval" : "";
  auto learn_ptr = eval ? learn_eval : predict_or_learn<true>;
  auto predict_ptr = eval ? predict_eval : predict_or_learn<false>;
  auto label_type = eval ? VW::label_type_t::CB_EVAL : VW::label_type_t::CB;
  VW::learner_update_stats_func<cb, VW::example>* update_stats_func =
      eval ? update_stats_cb_algs<true> : update_stats_cb_algs<false>;
  VW::learner_output_example_prediction_func<cb, VW::example>* output_example_prediction_func =
      eval ? output_example_prediction_cb_algs<true> : output_example_prediction_cb_algs<false>;
  VW::learner_print_update_func<cb, VW::example>* print_update_func =
      eval ? print_update_cb_algs<true> : print_update_cb_algs<false>;

  auto l = make_reduction_learner(
      std::move(data), base, learn_ptr, predict_ptr, stack_builder.get_setupfn_name(cb_algs_setup) + name_addition)
               .set_input_label_type(label_type)
               .set_output_label_type(VW::label_type_t::CS)
               .set_input_prediction_type(VW::prediction_type_t::MULTICLASS)
               .set_output_prediction_type(VW::prediction_type_t::MULTICLASS)
               .set_params_per_weight(problem_multiplier)
               .set_learn_returns_prediction(eval)
               .set_update_stats(update_stats_func)
               .set_output_example_prediction(output_example_prediction_func)
               .set_print_update(print_update_func)
               .build();

  return l;
}
