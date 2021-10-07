// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "action_score.h"
#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#include <cfloat>

#include "distributionally_robust.h"
#include "io/logger.h"
#include "vw.h"
#include "example.h"

#include <memory>
#include <utility>
#include <algorithm>

using namespace VW;
using namespace VW::config;
using namespace VW::LEARNER;

namespace logger = VW::io::logger;

namespace VW
{
struct discounted_expectation {
  discounted_expectation(double tau): tau(tau), sum(0), n(0) { }

  void update(double w, double r) {
    sum = tau * sum + w * r;
    n = tau * n + w;
  }

  double current() const {
    return n == 0 ? 0 : sum / n;
  }

  void save_load(io_buf& io, bool read, bool text, const char* name) {
    std::stringstream msg;
    if (!read) {
      msg << name << "_expectation_sum " << std::fixed << sum << "\n";
    }
    bin_text_read_write_fixed_validated(
        io, reinterpret_cast<char*>(&sum), sizeof(sum), "", read, msg, text);

    if (!read) {
      msg << name << "_expectation_n " << std::fixed << n << "\n";
    }
    bin_text_read_write_fixed_validated(
        io, reinterpret_cast<char*>(&n), sizeof(sum), "", read, msg, text);
  }
private:
  double tau, sum, n;
};

constexpr double DEFAULT_TAU = 0.999;
constexpr double DEFAULT_ALPHA = 0.05;

struct baseline_challenger_data {
  distributionally_robust::ChiSquared baseline;
  discounted_expectation policy_expectation;
  float baseline_epsilon;
  bool emit_metrics;

  baseline_challenger_data(bool emit_metrics, float alpha, float tau) :
      baseline(alpha, tau),
      policy_expectation(tau),
      emit_metrics(emit_metrics)
  {
  }

  static int get_chosen_action(const ACTION_SCORE::action_scores& action_scores)
  {
    return action_scores[0].action;
  }

  template <bool is_learn>
  inline void learn_or_predict(multi_learner& base, multi_ex& examples)
  {
    multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

    //must not be const since we mutate it at the end
    auto& action_scores = examples[0]->pred.a_s;
    const uint32_t chosen_action = get_chosen_action(action_scores);

    if (is_learn)
    {
      const auto lbl_example = std::find_if(examples.begin(), examples.end(), [](example* item) { return !item->l.cb.costs.empty(); });
      const auto labelled_action = static_cast<uint32_t>(std::distance(examples.begin(), lbl_example));
      if (lbl_example != examples.end()) {
        const CB::cb_class logged = (*lbl_example)->l.cb.costs[0];

        double r = -logged.cost;
        double w = (labelled_action == chosen_action ? 1 : 0) / logged.probability;
        double w2 = (labelled_action == 0 ? 1 : 0) / logged.probability;

        policy_expectation.update(w, r);
        baseline.update(w2, r);
      }

      multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
    }

    //We play baseline if policy expectation is worse than the baseline lower bound
    double ci = baseline.lower_bound();
    double exp = policy_expectation.current();

    //TODO don't check for it at every time step
    bool play_baseline = ci > exp;

    if(play_baseline) {
      for(auto& it : action_scores) {
        if (it.action == 0) {
          it.action = chosen_action;
        } else if (it.action == chosen_action) {
          it.action = 0;
        }
      }
    }
  }

};

} // namespace VW

template <bool is_learn>
void learn_or_predict(baseline_challenger_data& data, multi_learner& base, multi_ex& examples)
{
  data.learn_or_predict<is_learn>(base, examples);
}

void save_load(baseline_challenger_data& data, io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }

  data.policy_expectation.save_load(io, read, text, "policy");
  data.baseline.save_load(io, read, text, "baseline");
}

void persist_metrics(baseline_challenger_data& data, metric_sink& metrics)
{
  if (!data.emit_metrics) {
    return;
  }
  double ci = data.baseline.lower_bound();
  double exp = data.policy_expectation.current();

  metrics.float_metrics_list.emplace_back("baseline_cb_baseline_lowerbound", ci);
  metrics.float_metrics_list.emplace_back("baseline_cb_policy_expectation", exp);
  metrics.int_metrics_list.emplace_back("baseline_cb_baseline_in_use", ci > exp);
}

VW::LEARNER::base_learner* baseline_challenger_cb_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();
  double alpha;
  double tau;
  bool is_enabled = false;

  option_group_definition new_options("Baseline challenger reduction: Build a CI around the baseline action and use it instead of the model if it's perfoming better");
  new_options.add(make_option("baseline_challenger_cb", is_enabled).necessary().keep().help("Enable reduction"))
      .add(make_option("cb_c_alpha", alpha).default_value(DEFAULT_ALPHA).keep().help("Confidence level for "))
      .add(make_option("cb_c_tau", tau).default_value(DEFAULT_TAU).keep().help("Time constant for count decay"));

  if (!options.add_parse_and_check_necessary(new_options)) {
    return nullptr;
  }

  if (!options.was_supplied("cb_adf"))
    { THROW("cb_challenger requires cb_explore_adf or cb_adf"); }

  if (!all.logger.quiet)
  {
    *(all.trace_message) << "Using CB baseline challenger" << std::endl;
  }

  bool emit_metrics = options.was_supplied("extra_metrics");
  auto data = VW::make_unique<baseline_challenger_data>(emit_metrics, alpha, tau);

  auto* l =
      make_reduction_learner(std::move(data),
          as_multiline(stack_builder.setup_base_learner()),
          learn_or_predict<true>,
          learn_or_predict<false>,
          stack_builder.get_setupfn_name(baseline_challenger_cb_setup))
      .set_prediction_type(prediction_type_t::action_scores)
      .set_label_type(label_type_t::cb)
      .set_save_load(save_load)
      .set_persist_metrics(persist_metrics)
      .build();
  return make_base(*l);
}

