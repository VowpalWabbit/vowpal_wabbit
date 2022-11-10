// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_algs.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/cb_label_parser.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>

using namespace VW::LEARNER;
using namespace VW::config;

using namespace CB;
using namespace GEN_CS;

namespace CB_ALGS
{
void generic_output_example(
    VW::workspace& all, float loss, const VW::example& ec, const CB::label& ld, const CB::cb_class* known_cost)
{
  all.sd->update(ec.test_only, !CB::is_test_label(ld), loss, 1.f, ec.get_num_features());

  for (auto& sink : all.final_prediction_sink)
  { all.print_by_ref(sink.get(), static_cast<float>(ec.pred.multiclass), 0, ec.tag, all.logger); }

  if (all.raw_prediction != nullptr)
  {
    std::stringstream output_string_stream;
    for (unsigned int i = 0; i < ld.costs.size(); i++)
    {
      cb_class cl = ld.costs[i];
      if (i > 0) { output_string_stream << ' '; }
      output_string_stream << cl.action << ':' << cl.partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, all.logger);
  }

  bool is_ld_test_label = CB::is_test_label(ld);
  if (!is_ld_test_label) { print_update(all, is_ld_test_label, ec, nullptr, false, known_cost); }
  else
  {
    print_update(all, is_ld_test_label, ec, nullptr, false, nullptr);
  }
}
}  // namespace CB_ALGS
namespace
{
class cb
{
public:
  cb_to_cs cbcs;
  VW::io::logger logger;

  cb(VW::io::logger logger) : logger(std::move(logger)) {}
};

template <bool is_learn>
void predict_or_learn(cb& data, single_learner& base, VW::example& ec)
{
  cb_to_cs& c = data.cbcs;
  auto optional_cost = get_observed_cost_cb(ec.l.cb);
  // cost observed, not default
  if (optional_cost.first) { c.known_cost = optional_cost.second; }
  else
  {
    c.known_cost = CB::cb_class{};
  }

  // cost observed, not default
  if (optional_cost.first && (c.known_cost.action < 1 || c.known_cost.action > c.num_actions))
  { data.logger.err_error("invalid action: {}", c.known_cost.action); }

  // generate a cost-sensitive example to update classifiers
  gen_cs_example<is_learn>(c, ec, ec.l.cb, ec.l.cs, data.logger);

  if (c.cb_type != VW::cb_type_t::DM)
  {
    if (is_learn) { base.learn(ec); }
    else
    {
      base.predict(ec);
    }

    for (size_t i = 0; i < ec.l.cb.costs.size(); i++)
    { ec.l.cb.costs[i].partial_prediction = ec.l.cs.costs[i].partial_prediction; }
  }
}

void predict_eval(cb&, single_learner&, VW::example&) { THROW("can not use a test label for evaluation"); }

void learn_eval(cb& data, single_learner&, VW::example& ec)
{
  cb_to_cs& c = data.cbcs;
  auto optional_cost = get_observed_cost_cb(ec.l.cb_eval.event);
  // cost observed, not default
  if (optional_cost.first) { c.known_cost = optional_cost.second; }
  else
  {
    c.known_cost = CB::cb_class{};
  }
  gen_cs_example<true>(c, ec, ec.l.cb_eval.event, ec.l.cs, data.logger);

  for (size_t i = 0; i < ec.l.cb_eval.event.costs.size(); i++)
  { ec.l.cb_eval.event.costs[i].partial_prediction = ec.l.cs.costs[i].partial_prediction; }

  ec.pred.multiclass = ec.l.cb_eval.action;
}

void output_example(VW::workspace& all, cb& data, const VW::example& ec, const CB::label& ld)
{
  float loss = 0.;

  cb_to_cs& c = data.cbcs;
  if (!CB::is_test_label(ld)) { loss = CB_ALGS::get_cost_estimate(c.known_cost, c.pred_scores, ec.pred.multiclass); }

  CB_ALGS::generic_output_example(all, loss, ec, ld, &c.known_cost);
}

void finish_example(VW::workspace& all, cb& c, VW::example& ec)
{
  output_example(all, c, ec, ec.l.cb);
  VW::finish_example(all, ec);
}

void eval_finish_example(VW::workspace& all, cb& c, VW::example& ec)
{
  output_example(all, c, ec, ec.l.cb_eval.event);
  VW::finish_example(all, ec);
}

struct options_cb_algs_v1
{
  uint32_t num_actions;
  std::string type_string = "dr";
  bool eval = false;
  bool force_legacy = true;
  size_t problem_multiplier = 2;  // default for DR
};

std::unique_ptr<options_cb_algs_v1> get_cb_algs_options_instance(
    const VW::workspace&, VW::io::logger& logger, options_i& options)
{
  auto cb_algs_opts = VW::make_unique<options_cb_algs_v1>();
  option_group_definition new_options("[Reduction] Contextual Bandit");
  new_options
      .add(make_option("cb", cb_algs_opts->num_actions)
               .keep()
               .necessary()
               .help("Use contextual bandit learning with <k> costs"))
      .add(make_option("cb_type", cb_algs_opts->type_string)
               .keep()
               .default_value("dr")
               .one_of({"ips", "dm", "dr", "mtr", "sm"})
               .help("Contextual bandit method to use"))
      .add(make_option("eval", cb_algs_opts->eval).help("Evaluate a policy rather than optimizing"))
      .add(make_option("cb_force_legacy", cb_algs_opts->force_legacy)
               .keep()
               .help("Default to non-adf cb implementation (cb_to_cb_adf)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (!cb_algs_opts->eval && !cb_algs_opts->force_legacy) { return nullptr; }

  // Ensure serialization of this option in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", cb_algs_opts->type_string);
    options.add_and_parse(new_options);
  }

  if (!options.was_supplied("csoaa"))
  {
    std::stringstream ss;
    ss << cb_algs_opts->num_actions;
    options.insert("csoaa", ss.str());
  }

  switch (VW::cb_type_from_string(cb_algs_opts->type_string))
  {
    case VW::cb_type_t::DR:
      break;
    case VW::cb_type_t::DM:
      if (cb_algs_opts->eval) THROW("direct method can not be used for evaluation --- it is biased.");
      cb_algs_opts->problem_multiplier = 1;
      break;
    case VW::cb_type_t::IPS:
      cb_algs_opts->problem_multiplier = 1;
      break;
    case VW::cb_type_t::MTR:
    case VW::cb_type_t::SM:
      logger.err_warn(
          "cb_type must be in {{'ips','dm','dr'}}; resetting to dr. Input received: {}", cb_algs_opts->type_string);
      cb_algs_opts->type_string = "dr";
      break;
  }

  return cb_algs_opts;
}

}  // namespace

base_learner* VW::reductions::cb_algs_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto cb_algs_opts = get_cb_algs_options_instance(all, all.logger, options);
  if (cb_algs_opts == nullptr) { return nullptr; }
  auto cb_algs_data = VW::make_unique<cb>(all.logger);
  cb_algs_data->cbcs.num_actions = cb_algs_opts->num_actions;

  cb_to_cs& c = cb_algs_data->cbcs;

  c.cb_type = VW::cb_type_from_string(cb_algs_opts->type_string);

  auto base = as_singleline(stack_builder.setup_base_learner());
  if (cb_algs_opts->eval) { all.example_parser->lbl_parser = CB_EVAL::cb_eval; }
  else
  {
    all.example_parser->lbl_parser = CB::cb_label;
  }
  c.scorer = VW::LEARNER::as_singleline(base->get_learner_by_name_prefix("scorer"));

  std::string name_addition = cb_algs_opts->eval ? "-eval" : "";
  auto learn_ptr = cb_algs_opts->eval ? learn_eval : predict_or_learn<true>;
  auto predict_ptr = cb_algs_opts->eval ? predict_eval : predict_or_learn<false>;
  auto label_type = cb_algs_opts->eval ? VW::label_type_t::CB_EVAL : VW::label_type_t::CB;
  auto finish_ex = cb_algs_opts->eval ? eval_finish_example : ::finish_example;

  auto* l = make_reduction_learner(std::move(cb_algs_data), base, learn_ptr, predict_ptr,
      stack_builder.get_setupfn_name(cb_algs_setup) + name_addition)
                .set_input_label_type(label_type)
                .set_output_label_type(VW::label_type_t::CS)
                .set_input_prediction_type(VW::prediction_type_t::MULTICLASS)
                .set_output_prediction_type(VW::prediction_type_t::MULTICLASS)
                .set_params_per_weight(cb_algs_opts->problem_multiplier)
                .set_learn_returns_prediction(cb_algs_opts->eval)
                .set_finish_example(finish_ex)
                .build(&all.logger);

  return make_base(*l);
}
