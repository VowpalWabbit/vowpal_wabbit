// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <algorithm>
#include <cstdint>

// Most of these includes are required because templated functions are using the objects defined in them
// A few options to get rid of them:
// - Use virtual function calls in predict/learn to get rid of the templates entirely (con: virtual function calls)
// - Cut out the portions of code that actually use the objects and put them into new functions
//   defined in the cc file (con: can't inline those functions)
// - templatize all input parameters (con: no type safety)
#include "vw/core/action_score.h"    // used in sort_action_probs
#include "vw/core/cb.h"              // required for VW::cb_label
#include "vw/core/example.h"         // used in predict
#include "vw/core/gen_cs_example.h"  // required for VW::details::cb_to_cs_adf
#include "vw/core/global_data.h"
#include "vw/core/metric_sink.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/cb/cb_adf.h"  // used for function call in predict/learn
#include "vw/core/shared_data.h"
#include "vw/core/v_array.h"  // required by action_score.h
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"
#include "vw/core/vw_math.h"

#include <memory>

namespace VW
{
namespace cb_explore_adf
{
// Free functions
inline void sort_action_probs(v_array<VW::action_score>& probs, const std::vector<float>& scores)
{
  // We want to preserve the score order in the returned action_probs if possible.  To do this,
  // sort top_actions and action_probs by the order induced in scores.
  std::sort(probs.begin(), probs.end(),
      [&scores](const VW::action_score& as1, const VW::action_score& as2)
      {
        if (as1.score > as2.score) { return true; }
        else if (as1.score < as2.score) { return false; }
        // equal probabilities
        if (scores[as1.action] < scores[as2.action]) { return true; }
        else if (scores[as1.action] > scores[as2.action]) { return false; }
        // equal probabilities and equal cost estimates
        return as1.action < as2.action;
      });
}

inline size_t fill_tied(const v_array<VW::action_score>& preds)
{
  if (preds.size() == 0) { return 0; }

  size_t ret = 1;
  for (size_t i = 1; i < preds.size(); ++i)
  {
    if (VW::math::are_same_rel(preds[i].score, preds[0].score)) { ++ret; }
    else { return ret; }
  }
  return ret;
}

class cb_explore_metrics
{
public:
  size_t metric_labeled = 0;
  size_t metric_predict_in_learn = 0;
  float metric_sum_cost = 0.0;
  float metric_sum_cost_first = 0.0;
  size_t label_action_first_option = 0;
  size_t label_action_not_first = 0;
  size_t count_non_zero_cost = 0;
  size_t sum_features = 0;
  size_t sum_actions = 0;
  size_t min_actions = SIZE_MAX;
  size_t max_actions = 0;
  size_t sum_namespaces = 0;
};

// Object
template <typename ExploreType>
// data common to all cb_explore_adf reductions
class cb_explore_adf_base
{
public:
  template <typename... Args>
  cb_explore_adf_base(bool with_metrics, Args&&... args) : explore(std::forward<Args>(args)...)
  {
    if (with_metrics) { _metrics = VW::make_unique<cb_explore_metrics>(); }
  }

  static void save_load(cb_explore_adf_base<ExploreType>& data, io_buf& io, bool read, bool text);
  static void persist_metrics(cb_explore_adf_base<ExploreType>& data, metric_sink& metrics);
  static void predict(cb_explore_adf_base<ExploreType>& data, VW::LEARNER::learner& base, multi_ex& examples);
  static void learn(cb_explore_adf_base<ExploreType>& data, VW::LEARNER::learner& base, multi_ex& examples);

  static void update_stats(const VW::workspace& all, VW::shared_data& sd, const cb_explore_adf_base<ExploreType>& data,
      const multi_ex& ec_seq, VW::io::logger& logger);
  static void output_example_prediction(
      VW::workspace& all, const cb_explore_adf_base<ExploreType>& data, const multi_ex& ec_seq, VW::io::logger& logger);
  static void print_update(VW::workspace& all, VW::shared_data& sd, const cb_explore_adf_base<ExploreType>& data,
      const multi_ex& ec_seq, VW::io::logger& logger);

  ExploreType explore;

private:
  VW::cb_class _known_cost;
  // used in output_example
  VW::cb_label _action_label;
  VW::cb_label _empty_label;
  VW::action_scores _saved_pred;
  std::unique_ptr<cb_explore_metrics> _metrics;

  void _update_stats(
      const VW::workspace& all, VW::shared_data& sd, const multi_ex& ec_seq, VW::io::logger& logger) const;
  void _output_example_prediction(VW::workspace& all, const multi_ex& ec_seq, VW::io::logger& logger) const;
  void _print_update(VW::workspace& all, VW::shared_data& sd, const multi_ex& ec_seq, VW::io::logger& logger) const;
};

template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::predict(
    cb_explore_adf_base<ExploreType>& data, VW::LEARNER::learner& base, multi_ex& examples)
{
  example* label_example = VW::test_cb_adf_sequence(examples);
  data._known_cost = VW::get_observed_cost_or_default_cb_adf(examples);

  if (label_example != nullptr)
  {
    // predict path, replace the label example with an empty one
    data._action_label = std::move(label_example->l.cb);
    label_example->l.cb = std::move(data._empty_label);
  }

  data.explore.predict(base, examples);

  if (label_example != nullptr)
  {
    // predict path, restore label
    label_example->l.cb = std::move(data._action_label);
    data._empty_label.costs.clear();
    data._empty_label.weight = 1.f;
  }
}

template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::learn(
    cb_explore_adf_base<ExploreType>& data, VW::LEARNER::learner& base, multi_ex& examples)
{
  example* label_example = VW::test_cb_adf_sequence(examples);
  if (label_example != nullptr)
  {
    data._known_cost = VW::get_observed_cost_or_default_cb_adf(examples);
    // learn iff label_example != nullptr
    data.explore.learn(base, examples);
    if (data._metrics)
    {
      data._metrics->metric_labeled++;
      data._metrics->metric_sum_cost += data._known_cost.cost;
      if (data._known_cost.action == 0)
      {
        data._metrics->label_action_first_option++;
        data._metrics->metric_sum_cost_first += data._known_cost.cost;
      }
      else { data._metrics->label_action_not_first++; }

      if (data._known_cost.cost != 0) { data._metrics->count_non_zero_cost++; }

      data._metrics->sum_actions += examples.size();
      data._metrics->max_actions = std::max(data._metrics->max_actions, examples.size());
      data._metrics->min_actions = std::min(data._metrics->min_actions, examples.size());
    }
  }
  else
  {
    predict(data, base, examples);
    if (data._metrics) { data._metrics->metric_predict_in_learn++; }
  }
}

template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::update_stats(const VW::workspace& all, VW::shared_data& sd,
    const cb_explore_adf_base<ExploreType>& data, const multi_ex& ec_seq, VW::io::logger& logger)
{
  data._update_stats(all, sd, ec_seq, logger);
}
template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::output_example_prediction(
    VW::workspace& all, const cb_explore_adf_base<ExploreType>& data, const multi_ex& ec_seq, VW::io::logger& logger)
{
  data._output_example_prediction(all, ec_seq, logger);
}
template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::print_update(VW::workspace& all, VW::shared_data& sd,
    const cb_explore_adf_base<ExploreType>& data, const multi_ex& ec_seq, VW::io::logger& logger)
{
  data._print_update(all, sd, ec_seq, logger);
}

template <typename ExploreType>
void cb_explore_adf_base<ExploreType>::_update_stats(
    const VW::workspace& /*all*/, VW::shared_data& sd, const multi_ex& ec_seq, VW::io::logger& /*logger*/
) const
{
  if (ec_seq.size() <= 0) { return; }

  size_t num_features = 0;
  size_t num_namespaces = 0;

  float loss = 0.;

  auto& ec = *ec_seq[0];
  const auto& preds = ec.pred.a_s;

  for (const auto& example : ec_seq)
  {
    if (VW::ec_is_example_header_cb(*example))
    {
      num_features += (ec_seq.size() - 1) *
          (example->get_num_features() - example->feature_space[VW::details::CONSTANT_NAMESPACE].size());
      num_namespaces += (ec_seq.size() - 1) * example->indices.size();
    }
    else
    {
      num_features += example->get_num_features();
      num_namespaces += example->indices.size();
    }
  }

  if (_metrics)
  {
    _metrics->sum_features += num_features;
    _metrics->sum_namespaces += num_namespaces;
  }

  bool labeled_example = true;
  if (_known_cost.probability > 0)
  {
    for (uint32_t i = 0; i < preds.size(); i++)
    {
      float l = VW::get_cost_estimate(_known_cost, preds[i].action);
      loss += l * preds[i].score * ec_seq[ec_seq.size() - preds.size() + i]->weight;
    }
  }
  else { labeled_example = false; }

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq.size(); i++) { holdout_example &= ec_seq[i]->test_only; }

  sd.update(holdout_example, labeled_example, loss, ec.weight, num_features);
}

template <typename ExploreType>
void cb_explore_adf_base<ExploreType>::_output_example_prediction(
    VW::workspace& all, const multi_ex& ec_seq, VW::io::logger& logger) const
{
  if (ec_seq.size() <= 0) { return; }
  auto& ec = *ec_seq[0];
  for (auto& sink : all.final_prediction_sink)
  {
    VW::details::print_action_score(sink.get(), ec.pred.a_s, ec.tag, logger);
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
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, logger);
  }
  // maintain legacy printing behavior
  if (all.raw_prediction != nullptr) { all.print_text_by_ref(all.raw_prediction.get(), "", ec_seq[0]->tag, logger); }
  VW::details::global_print_newline(all.final_prediction_sink, logger);
}

template <typename ExploreType>
void cb_explore_adf_base<ExploreType>::_print_update(
    VW::workspace& all, VW::shared_data& /*sd*/, const multi_ex& ec_seq, VW::io::logger& /*logger*/
) const
{
  if (ec_seq.size() <= 0) { return; }
  bool labeled_example = (_known_cost.probability > 0);
  auto& ec = *ec_seq[0];
  if (labeled_example) { VW::details::print_update_cb(all, !labeled_example, ec, &ec_seq, true, &_known_cost); }
  else { VW::details::print_update_cb(all, !labeled_example, ec, &ec_seq, true, nullptr); }
}

template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::save_load(
    cb_explore_adf_base<ExploreType>& data, io_buf& io, bool read, bool text)
{
  data.explore.save_load(io, read, text);
}

template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::persist_metrics(
    cb_explore_adf_base<ExploreType>& data, metric_sink& metrics)
{
  if (data._metrics)
  {
    metrics.set_uint("cbea_labeled_ex", data._metrics->metric_labeled);
    metrics.set_uint("cbea_predict_in_learn", data._metrics->metric_predict_in_learn);
    metrics.set_float("cbea_sum_cost", data._metrics->metric_sum_cost);
    metrics.set_float("cbea_sum_cost_baseline", data._metrics->metric_sum_cost_first);
    metrics.set_uint("cbea_label_first_action", data._metrics->label_action_first_option);
    metrics.set_uint("cbea_label_not_first", data._metrics->label_action_not_first);
    metrics.set_uint("cbea_non_zero_cost", data._metrics->count_non_zero_cost);

    if (data._metrics->metric_labeled)
    {
      metrics.set_float(
          "cbea_avg_feat_per_event", static_cast<float>(data._metrics->sum_features / data._metrics->metric_labeled));
      metrics.set_float(
          "cbea_avg_actions_per_event", static_cast<float>(data._metrics->sum_actions / data._metrics->metric_labeled));
      metrics.set_float(
          "cbea_avg_ns_per_event", static_cast<float>(data._metrics->sum_namespaces / data._metrics->metric_labeled));
    }

    if (data._metrics->sum_actions)
    {
      metrics.set_float(
          "cbea_avg_feat_per_action", static_cast<float>(data._metrics->sum_features / data._metrics->sum_actions));
      metrics.set_float(
          "cbea_avg_ns_per_action", static_cast<float>(data._metrics->sum_namespaces / data._metrics->sum_actions));
    }

    if (data._metrics->min_actions != SIZE_MAX) { metrics.set_uint("cbea_min_actions", data._metrics->min_actions); }

    if (data._metrics->max_actions) { metrics.set_uint("cbea_max_actions", data._metrics->max_actions); }
  }
}

}  // namespace cb_explore_adf
}  // namespace VW
