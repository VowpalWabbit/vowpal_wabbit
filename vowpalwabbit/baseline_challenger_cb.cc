// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "baseline_challenger_cb.h"

#include "action_score.h"
#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#include <cfloat>

#include "distributionally_robust.h"
#include "vw.h"
#include "example.h"
#include "model_utils.h"

#include <memory>
#include <utility>
#include <algorithm>

using namespace VW;
using namespace VW::config;
using namespace VW::LEARNER;

namespace VW
{
struct discounted_expectation;
struct baseline_challenger_data;
namespace model_utils
{
size_t read_model_field(io_buf&, VW::discounted_expectation&);
size_t write_model_field(io_buf&, const VW::discounted_expectation&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::baseline_challenger_data&);
size_t write_model_field(io_buf&, const VW::baseline_challenger_data&, const std::string&, bool);
}  // namespace model_utils
struct discounted_expectation
{
  discounted_expectation(double tau) : tau(tau), sum(0), n(0) {}

  void update(double w, double r)
  {
    sum = tau * sum + w * r;
    n = tau * n + w;
  }

  double current() const { return n == 0 ? 0 : sum / n; }

  friend size_t VW::model_utils::read_model_field(io_buf&, VW::discounted_expectation&);
  friend size_t VW::model_utils::write_model_field(
      io_buf&, const VW::discounted_expectation&, const std::string&, bool);

private:
  double tau;
  double sum;
  double n;
};

struct baseline_challenger_data
{
  distributionally_robust::ChiSquared baseline;
  discounted_expectation policy_expectation;
  float baseline_epsilon;
  bool emit_metrics;

  baseline_challenger_data(bool emit_metrics, double alpha, double tau)
      : baseline(alpha, tau), policy_expectation(tau), emit_metrics(emit_metrics)
  {
  }

  static int get_chosen_action(const ACTION_SCORE::action_scores& action_scores) { return action_scores[0].action; }

  template <bool is_learn>
  inline void learn_or_predict(multi_learner& base, multi_ex& examples)
  {
    multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

    // must not be const since we mutate it at the end
    auto& action_scores = examples[0]->pred.a_s;
    const uint32_t chosen_action = get_chosen_action(action_scores);

    if (is_learn)
    {
      const auto lbl_example =
          std::find_if(examples.begin(), examples.end(), [](example* item) { return !item->l.cb.costs.empty(); });
      if (lbl_example != examples.end())
      {
        const auto labelled_action = static_cast<uint32_t>(std::distance(examples.begin(), lbl_example));
        const CB::cb_class& logged = (*lbl_example)->l.cb.costs[0];

        double r = -logged.cost;
        double w = (labelled_action == chosen_action ? 1 : 0) / logged.probability;
        double w2 = (labelled_action == 0 ? 1 : 0) / logged.probability;

        policy_expectation.update(w, r);
        baseline.update(w2, r);
      }

      multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
    }

    // We play baseline if policy expectation is worse than the baseline lower bound
    double ci = baseline.lower_bound();
    double expectation = policy_expectation.current();

    // TODO don't check for it at every time step
    bool play_baseline = ci > expectation;

    if (play_baseline)
    {
      for (auto& it : action_scores)
      {
        if (it.action == 0) { it.action = chosen_action; }
        else if (it.action == chosen_action)
        {
          it.action = 0;
        }
      }
    }
  }

  friend size_t VW::model_utils::read_model_field(io_buf&, VW::baseline_challenger_data&);
  friend size_t VW::model_utils::write_model_field(
      io_buf&, const VW::baseline_challenger_data&, const std::string&, bool);
};

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::discounted_expectation& de)
{
  size_t bytes = 0;
  bytes += read_model_field(io, de.sum);
  bytes += read_model_field(io, de.n);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::discounted_expectation& de, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, de.sum, upstream_name + "_expectation_sum", text);
  bytes += write_model_field(io, de.n, upstream_name + "_expectation_n", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::baseline_challenger_data& challenger)
{
  size_t bytes = 0;
  bytes += read_model_field(io, challenger.baseline);
  bytes += read_model_field(io, challenger.policy_expectation);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::baseline_challenger_data& challenger, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, challenger.baseline, upstream_name + "_baseline", text);
  bytes += write_model_field(io, challenger.policy_expectation, upstream_name + "_policy", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW

template <bool is_learn>
void learn_or_predict(baseline_challenger_data& data, multi_learner& base, multi_ex& examples)
{
  data.learn_or_predict<is_learn>(base, examples);
}

void save_load(baseline_challenger_data& data, io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (read) { VW::model_utils::read_model_field(io, data); }
  else
  {
    VW::model_utils::write_model_field(io, data, "_challenger", text);
  }
}

void persist_metrics(baseline_challenger_data& data, metric_sink& metrics)
{
  if (!data.emit_metrics) { return; }
  auto ci = static_cast<float>(data.baseline.lower_bound());
  auto exp = static_cast<float>(data.policy_expectation.current());

  metrics.set_float("baseline_cb_baseline_lowerbound", ci);
  metrics.set_float("baseline_cb_policy_expectation", exp);
  metrics.set_bool("baseline_cb_baseline_in_use", ci > exp);
}

VW::LEARNER::base_learner* baseline_challenger_cb_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  float alpha;
  float tau;
  bool is_enabled = false;

  option_group_definition new_options("[Reduction] Baseline challenger");
  new_options
      .add(make_option("baseline_challenger_cb", is_enabled)
               .necessary()
               .keep()
               .help("Experimental: Build a CI around the baseline action and use it instead of the model if it's "
                     "perfoming better"))
      .add(make_option("cb_c_alpha", alpha).default_value(DEFAULT_ALPHA).keep().help("Confidence level for baseline"))
      .add(make_option("cb_c_tau", tau).default_value(DEFAULT_TAU).keep().help("Time constant for count decay"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (!options.was_supplied("cb_adf")) { THROW("cb_challenger requires cb_explore_adf or cb_adf"); }

  bool emit_metrics = options.was_supplied("extra_metrics");
  auto data = VW::make_unique<baseline_challenger_data>(emit_metrics, alpha, tau);

  auto* l = make_reduction_learner(std::move(data), as_multiline(stack_builder.setup_base_learner()),
      learn_or_predict<true>, learn_or_predict<false>, stack_builder.get_setupfn_name(baseline_challenger_cb_setup))
                .set_output_prediction_type(VW::prediction_type_t::action_scores)
                .set_input_label_type(VW::label_type_t::cb)
                .set_save_load(save_load)
                .set_persist_metrics(persist_metrics)
                .build();
  return make_base(*l);
}
