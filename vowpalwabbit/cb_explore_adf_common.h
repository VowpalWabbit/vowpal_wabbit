// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cstdint>
#include <algorithm>

// Most of these includes are required because templated functions are using the objects defined in them
// A few options to get rid of them:
// - Use virtual function calls in predict/learn to get rid of the templates entirely (con: virtual function calls)
// - Cut out the portions of code that actually use the objects and put them into new functions
//   defined in the cc file (con: can't inline those functions)
// - templatize all input parameters (con: no type safety)
#include "v_array.h"         // required by action_score.h
#include "action_score.h"    // used in sort_action_probs
#include "cb.h"              // required for CB::label
#include "cb_adf.h"          // used for function call in predict/learn
#include "example.h"         // used in predict
#include "gen_cs_example.h"  // required for GEN_CS::cb_to_cs_adf
#include "reductions_fwd.h"
#include "vw_math.h"
#include "shared_data.h"
#include "metric_sink.h"

namespace VW
{
namespace cb_explore_adf
{
// Free functions
inline void sort_action_probs(v_array<ACTION_SCORE::action_score>& probs, const std::vector<float>& scores)
{
  // We want to preserve the score order in the returned action_probs if possible.  To do this,
  // sort top_actions and action_probs by the order induced in scores.
  std::sort(probs.begin(), probs.end(),
      [&scores](const ACTION_SCORE::action_score& as1, const ACTION_SCORE::action_score& as2) {
        if (as1.score > as2.score)
          return true;
        else if (as1.score < as2.score)
          return false;
        // equal probabilities
        if (scores[as1.action] < scores[as2.action])
          return true;
        else if (scores[as1.action] > scores[as2.action])
          return false;
        // equal probabilities and equal cost estimates
        return as1.action < as2.action;
      });
}

inline size_t fill_tied(const v_array<ACTION_SCORE::action_score>& preds)
{
  if (preds.size() == 0) { return 0; }

  size_t ret = 1;
  for (size_t i = 1; i < preds.size(); ++i)
  {
    if (VW::math::are_same_rel(preds[i].score, preds[0].score)) { ++ret; }
    else
    {
      return ret;
    }
  }
  return ret;
}

struct cb_explore_metrics
{
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
struct cb_explore_adf_base
{
private:
  CB::cb_class _known_cost;
  // used in output_example
  CB::label _action_label;
  CB::label _empty_label;
  ACTION_SCORE::action_scores _saved_pred;
  std::unique_ptr<cb_explore_metrics> _metrics;

public:
  template <typename... Args>
  cb_explore_adf_base(bool with_metrics, Args&&... args) : explore(std::forward<Args>(args)...)
  {
    if (with_metrics) _metrics = VW::make_unique<cb_explore_metrics>();
  }

  static void finish_multiline_example(vw& all, cb_explore_adf_base<ExploreType>& data, multi_ex& ec_seq);
  static void print_multiline_example(vw& all, cb_explore_adf_base<ExploreType>& data, multi_ex& ec_seq);
  static void save_load(cb_explore_adf_base<ExploreType>& data, io_buf& io, bool read, bool text);
  static void persist_metrics(cb_explore_adf_base<ExploreType>& data, metric_sink& metrics);
  static void predict(cb_explore_adf_base<ExploreType>& data, VW::LEARNER::multi_learner& base, multi_ex& examples);
  static void learn(cb_explore_adf_base<ExploreType>& data, VW::LEARNER::multi_learner& base, multi_ex& examples);

public:
  ExploreType explore;

private:
  void output_example_seq(vw& all, multi_ex& ec_seq);
  void output_example(vw& all, multi_ex& ec_seq);
};

template <typename ExploreType>
inline void cb_explore_adf_base<ExploreType>::predict(
    cb_explore_adf_base<ExploreType>& data, VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  example* label_example = CB_ADF::test_adf_sequence(examples);
  data._known_cost = CB_ADF::get_observed_cost_or_default_cb_adf(examples);

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
    cb_explore_adf_base<ExploreType>& data, VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  example* label_example = CB_ADF::test_adf_sequence(examples);
  if (label_example != nullptr)
  {
    data._known_cost = CB_ADF::get_observed_cost_or_default_cb_adf(examples);
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
      else
      {
        data._metrics->label_action_not_first++;
      }

      if (data._known_cost.cost != 0) { data._metrics->count_non_zero_cost++; }

      data._metrics->sum_actions += examples.size();
      data._metrics->max_actions = std::max(data._metrics->max_actions, examples.size());
      data._metrics->min_actions = std::min(data._metrics->min_actions, examples.size());
    }
  }
  else
  {
    predict(data, base, examples);
    if (data._metrics) data._metrics->metric_predict_in_learn++;
  }
}

template <typename ExploreType>
void cb_explore_adf_base<ExploreType>::output_example(vw& all, multi_ex& ec_seq)
{
  if (ec_seq.size() <= 0) return;

  size_t num_features = 0;
  size_t num_namespaces = 0;

  float loss = 0.;

  auto& ec = *ec_seq[0];
  const auto& preds = ec.pred.a_s;

  for (const auto& example : ec_seq)
  {
    num_features += example->get_num_features();
    num_namespaces += example->indices.size();
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
      float l = CB_ALGS::get_cost_estimate(_known_cost, preds[i].action);
      loss += l * preds[i].score;
    }
  }
  else
    labeled_example = false;

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq.size(); i++) holdout_example &= ec_seq[i]->test_only;

  all.sd->update(holdout_example, labeled_example, loss, ec.weight, num_features);

  for (auto& sink : all.final_prediction_sink) ACTION_SCORE::print_action_score(sink.get(), ec.pred.a_s, ec.tag);

  if (all.raw_prediction != nullptr)
  {
    std::string outputString;
    std::stringstream outputStringStream(outputString);
    const auto& costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), outputStringStream.str(), ec.tag);
  }

  if (labeled_example)
    CB::print_update(all, !labeled_example, ec, &ec_seq, true, &_known_cost);
  else
    CB::print_update(all, !labeled_example, ec, &ec_seq, true, nullptr);
}

template <typename ExploreType>
void cb_explore_adf_base<ExploreType>::output_example_seq(vw& all, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example(all, ec_seq);
    if (all.raw_prediction != nullptr) all.print_text_by_ref(all.raw_prediction.get(), "", ec_seq[0]->tag);
  }
}

template <typename ExploreType>
void cb_explore_adf_base<ExploreType>::finish_multiline_example(
    vw& all, cb_explore_adf_base<ExploreType>& data, multi_ex& ec_seq)
{
  print_multiline_example(all, data, ec_seq);

  VW::finish_example(all, ec_seq);
}

template <typename ExploreType>
void cb_explore_adf_base<ExploreType>::print_multiline_example(
    vw& all, cb_explore_adf_base<ExploreType>& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    data.output_example_seq(all, ec_seq);
    CB_ADF::global_print_newline(all.final_prediction_sink);
  }
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
