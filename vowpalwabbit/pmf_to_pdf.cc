// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions.h"
#include "pmf_to_pdf.h"
#include "explore.h"
#include "guard.h"
#include "vw.h"
#include "cb_label_parser.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

namespace VW
{
namespace pmf_to_pdf
{
void reduction::transform_prediction(example& ec)
{
  const float continuous_range = max_value - min_value;
  const float unit_range = continuous_range / num_actions;

  size_t n = temp_pred_a_s.size();
  assert(n != 0);

  auto score = temp_pred_a_s[0].score;
  // map discrete action (predicted tree leaf) to the continuous value of the centre of the leaf
  auto centre = min_value + temp_pred_a_s[0].action * unit_range + unit_range / 2.0f;

  // if zero bandwidth -> stay inside leaf by smoothing around unit_range / 2 (leaf range is unit_range)
  auto b = !bandwidth ? unit_range / 2.0f : bandwidth;

  pdf_lim.clear();
  if (centre - b != min_value) pdf_lim.push_back(min_value);

  uint32_t l = 0;
  uint32_t r = 0;
  while (l < n || r < n)
  {
    if (centre >= b)
    {
      if (l == n || centre + b < centre - b)
      {
        auto val = std::min(centre + b, max_value);
        pdf_lim.push_back(val);
        r++;
      }
      else if (r == n || centre - b < centre + b)
      {
        auto val = std::max(centre - b, min_value);
        if ((!pdf_lim.empty() && pdf_lim.back() != val) || pdf_lim.empty()) { pdf_lim.push_back(val); }
        l++;
      }
      else if (centre - b == centre + b)
      {
        auto val = std::max(centre - b, min_value);
        if ((!pdf_lim.empty() && pdf_lim.back() != val) || pdf_lim.empty()) { pdf_lim.push_back(val); }
        l++;
        r++;
      }
    }
    else
    {
      // centre < b so lower limit should be min_value (already added to pdf_lim)
      // so need to add centre + b
      auto val = std::min(centre + b, max_value);
      pdf_lim.push_back(val);
      l++;
      r++;
    }
  }

  if (pdf_lim.back() != max_value) pdf_lim.push_back(max_value);

  auto& p_dist = ec.pred.pdf;
  p_dist.clear();

  size_t m = pdf_lim.size();
  l = 0;
  for (uint32_t i = 0; i < m - 1; i++)
  {
    float p = 0;
    // there are 2 ways of knowing that we are entering the pdf limits of the chosen action and thus need to assign a
    // probability: (1) if centre - b < min_value -> pdf_lim would be 'min_value' or
    // (2) pdf_lim is 'centre - b'
    if (l < n && (((centre - min_value) < b && pdf_lim[i] == min_value) || pdf_lim[i] == centre - b))
    {
      // default: 2 * b : 'centre - b' to 'centre + b'
      float actual_b = std::min(max_value, centre + b) - std::max(min_value, centre - b);
      p += score / actual_b;
      l++;
    }
    const float left = pdf_lim[i];
    const float right = pdf_lim[i + 1];

    p_dist.push_back({left, right, p});
  }
}

reduction::~reduction()
{
  temp_lbl_cb.costs.delete_v();
  temp_pred_a_s.delete_v();
}

void reduction::predict(example& ec)
{
  auto swap_label = VW::swap_guard(ec.l.cb, temp_lbl_cb);

  const auto& reduction_features = ec._reduction_features.template get<VW::continuous_actions::reduction_features>();
  if (first_only && reduction_features.is_chosen_action_set())
  {
    float chosen_action = reduction_features.chosen_action;
    const float continuous_range = max_value - min_value;
    const float unit_range = continuous_range / num_actions;

    // discretize chosen action
    const float ac = (chosen_action - min_value) / unit_range;
    auto action = static_cast<uint32_t>(floor(ac));

    temp_pred_a_s.clear();
    temp_pred_a_s.push_back({action, 1.f});
  }
  else
  {
    // scope for saving / restoring prediction
    auto save_prediction = VW::swap_guard(ec.pred.a_s, temp_pred_a_s);
    _p_base->predict(ec);
  }
  transform_prediction(ec);
}

void reduction::learn(example& ec)
{
  const float cost = ec.l.cb_cont.costs[0].cost;
  const float pdf_value = ec.l.cb_cont.costs[0].pdf_value;
  const float action_cont = ec.l.cb_cont.costs[0].action;

  const float continuous_range = max_value - min_value;
  const float unit_range = continuous_range / num_actions;

  const float ac = (action_cont - min_value) / unit_range;
  int action_segment_index = static_cast<int>(floor(ac));
  const bool cond1 = min_value + action_segment_index * unit_range <= action_cont;
  const bool cond2 = action_cont < min_value + (action_segment_index + 1) * unit_range;

  if (!cond1 || !cond2)
  {
    if (!cond1) action_segment_index--;
    if (!cond2) action_segment_index++;
  }

  // going to pass label into tree, so need to used discretized version of bandwidth i.e. tree_bandwidth
  uint32_t b = tree_bandwidth;
  const uint32_t local_min_value = std::max((int)b, action_segment_index - (int)b + 1);
  const uint32_t local_max_value = std::min(num_actions - 1 - b, action_segment_index + b);

  auto swap_label = VW::swap_guard(ec.l.cb, temp_lbl_cb);

  ec.l.cb.costs.clear();

  auto actual_bandwidth = !tree_bandwidth ? 1 : 2 * b;  // avoid zero division

  ec.l.cb.costs.push_back(
      {cost, local_min_value + 1, pdf_value * actual_bandwidth * continuous_range / num_actions, 0.0f});
  ec.l.cb.costs.push_back(
      {cost, local_max_value + 1, pdf_value * actual_bandwidth * continuous_range / num_actions, 0.0f});

  auto swap_prediction = VW::swap_guard(ec.pred.a_s, temp_pred_a_s);

  _p_base->learn(ec);
}

void predict(pmf_to_pdf::reduction& data, single_learner&, example& ec) { data.predict(ec); }

void learn(pmf_to_pdf::reduction& data, single_learner&, example& ec) { data.learn(ec); }

void print_update(vw& all, bool is_test, example& ec, std::stringstream& pred_string)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.logger.quiet && !all.bfgs)
  {
    std::stringstream label_string;
    if (is_test)
      label_string << " unknown";
    else
    {
      const auto& cost = ec.l.cb.costs[0];
      label_string << cost.action << ":" << cost.cost << ":" << cost.probability;
    }
    all.sd->print_update(all.holdout_set_off, all.current_pass, label_string.str(), pred_string.str(), ec.num_features,
        all.progress_add, all.progress_arg);
  }
}

inline bool observed_cost(CB::cb_class* cl)
{
  // cost observed for this action if it has non zero probability and cost != FLT_MAX
  return (cl != nullptr && cl->cost != FLT_MAX && cl->probability > .0);
}

CB::cb_class* get_observed_cost(CB::label& ld)
{
  for (auto& cl : ld.costs)
    if (observed_cost(&cl)) return &cl;
  return nullptr;
}

void output_example(vw& all, reduction&, example& ec, CB::label& ld)
{
  float loss = 0.;

  if (get_observed_cost(ec.l.cb) != nullptr)
    for (auto& cbc : ec.l.cb.costs)
      for (uint32_t i = 0; i < ec.pred.pdf.size(); i++) loss += (cbc.cost / cbc.probability) * ec.pred.pdf[i].pdf_value;

  all.sd->update(ec.test_only, get_observed_cost(ld) != nullptr, loss, 1.f, ec.num_features);

  constexpr size_t buffsz = 20;
  char temp_str[buffsz];
  std::stringstream ss, sso;
  float maxprob = 0.;
  uint32_t maxid = 0;
  for (uint32_t i = 0; i < ec.pred.pdf.size(); i++)
  {
    sprintf_s(temp_str, buffsz, "%f ", ec.pred.pdf[i].pdf_value);
    ss << temp_str;
    if (ec.pred.pdf[i].pdf_value > maxprob)
    {
      maxprob = ec.pred.pdf[i].pdf_value;
      maxid = i + 1;
    }
  }

  sprintf_s(temp_str, buffsz, "%d:%f", maxid, maxprob);
  sso << temp_str;

  for (auto& sink : all.final_prediction_sink) all.print_text_by_ref(sink.get(), ss.str(), ec.tag);

  print_update(all, CB::is_test_label(ld), ec, sso);
}

void finish_example(vw& all, reduction& c, example& ec)
{
  output_example(all, c, ec, ec.l.cb);
  VW::finish_example(all, ec);
}

base_learner* setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<pmf_to_pdf::reduction>();

  option_group_definition new_options("Convert discrete PDF into continuous PDF");
  new_options
      .add(make_option("pmf_to_pdf", data->num_actions)
               .default_value(0)
               .necessary()
               .keep()
               .help("number of tree labels <k> for pmf_to_pdf"))
      .add(make_option("min_value", data->min_value).keep().help("Minimum continuous value"))
      .add(make_option("max_value", data->max_value).keep().help("Maximum continuous value"))
      .add(make_option("bandwidth", data->bandwidth)
               .default_value(1)
               .keep()
               .help("Bandwidth (radius) of randomization around discrete actions in terms of continuous range."))
      .add(make_option("first_only", data->first_only)
               .keep()
               .help("Use user provided first action or user provided pdf or uniform random"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (data->num_actions == 0) return nullptr;
  if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
  { THROW("error: min and max values must be supplied with cb_continuous"); }
  if (data->bandwidth <= -1) { THROW("error: Bandwidth must be positive"); }
  // Translate user provided bandwidth which is in terms of continuous action range (max_value - min_value)
  // to the internal tree bandwidth which is in terms of #actions
  float leaf_width = (data->max_value - data->min_value) / (data->num_actions);  // aka unit range
  float half_leaf_width = leaf_width / 2.f;

  if (data->bandwidth <= half_leaf_width) { data->tree_bandwidth = 0; }
  else if (std::fmod((data->bandwidth), leaf_width) == 0)
  {
    data->tree_bandwidth = ((data->bandwidth) / leaf_width);
  }
  else
  {
    data->tree_bandwidth = ((data->bandwidth) / leaf_width) + 1;
  }

  options.replace("tree_bandwidth", std::to_string(data->tree_bandwidth));

  auto p_base = as_singleline(setup_base(options, all));
  data->_p_base = p_base;

  learner<pmf_to_pdf::reduction, example>& l = init_learner(data, p_base, learn, predict, 1, prediction_type_t::pdf);

  all.delete_prediction = continuous_actions::delete_probability_density_function;

  return make_base(l);
}

}  // namespace pmf_to_pdf
}  // namespace VW
