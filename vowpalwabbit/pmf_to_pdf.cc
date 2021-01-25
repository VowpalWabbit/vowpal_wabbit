// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions.h"
#include "pmf_to_pdf.h"
#include "explore.h"
#include "guard.h"
#include "vw.h"

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
  auto centre = min_value + temp_pred_a_s[0].action * unit_range + unit_range / 2.0f;

  size_t n = temp_pred_a_s.size();
  assert(n != 0);

  struct as
  {
    float action;
    float score;
  };

  std::vector<as> pred_a_s;
  pred_a_s.push_back({centre, temp_pred_a_s[0].score});
  for (size_t i = 1; i < temp_pred_a_s.size(); i++)
  { pred_a_s.push_back({(float)temp_pred_a_s[i].action, temp_pred_a_s[i].score}); }

  auto b = !bandwidth ? 1 : bandwidth;  // TODO make better

  pdf_lim.clear();
  if (pred_a_s[0].action - b != min_value) pdf_lim.push_back(min_value);

  uint32_t l = 0;
  uint32_t r = 0;
  while (l < n || r < n)
  {
    if (pred_a_s[0].action >= b)
    {
      if (l == n || pred_a_s[r].action + b < pred_a_s[l].action - b)
      {
        auto val = std::min(pred_a_s[r++].action + b, max_value);
        pdf_lim.push_back(val);
      }
      else if (r == n || pred_a_s[l].action - b < pred_a_s[r].action + b)
      {
        auto val = std::max(pred_a_s[l++].action - b, min_value);
        if (pdf_lim.back() != val) { pdf_lim.push_back(val); }
      }
      else if (pred_a_s[l].action - b == pred_a_s[r].action + b)
      {
        auto val = std::max(pred_a_s[l].action - b, min_value);
        if (pdf_lim.back() != val) { pdf_lim.push_back(val); }
        l++;
        r++;
      }
    }
    else
    {
      // action - b < 0 so lower limit is min_value (already added to pdf_lim)
      auto val = std::min(pred_a_s[r++].action + b, max_value);
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
    if (l < n &&
        (((pred_a_s[l].action - min_value) < b && pdf_lim[i] == min_value) || pdf_lim[i] == pred_a_s[l].action - b))
    {
      // default: 2 * b : 'action - b' to 'action + b'
      float actual_b = std::min(max_value, pred_a_s[l].action + b) - std::max(min_value, pred_a_s[l].action - b);

      actual_b = !b ? 1 : actual_b;  // avoid zero division

      p += pred_a_s[l++].score / actual_b;
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

  auto b = !bandwidth ? 1 : bandwidth;
  const uint32_t local_min_value = (std::max)((int)b, action_segment_index - (int)b + 1);
  const uint32_t local_max_value = (std::min)(num_actions - b, action_segment_index + b);

  auto swap_label = VW::swap_guard(ec.l.cb, temp_lbl_cb);

  ec.l.cb.costs.clear();
  auto actual_bandwidth = !bandwidth ? 1 : 2 * bandwidth;
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

  print_update(all, CB::cb_label.test_label(&ld), ec, sso);
}

void finish_example(vw& all, reduction& c, example& ec)
{
  output_example(all, c, ec, ec.l.cb);
  VW::finish_example(all, ec);
}

base_learner* setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<pmf_to_pdf::reduction>();

  option_group_definition new_options("PMF to PDF");
  new_options
      .add(make_option("pmf_to_pdf", data->num_actions)
               .default_value(0)
               .necessary()
               .keep()
               .help("Convert discrete PDF into continuous PDF."))
      .add(make_option("min_value", data->min_value).keep().help("Minimum continuous value"))
      .add(make_option("max_value", data->max_value).keep().help("Maximum continuous value"))
      .add(make_option("bandwidth", data->bandwidth)
               .default_value(1)
               .keep()
               .help("Bandwidth (radius) of randomization around discrete actions in number of actions."))
      .add(make_option("first_only", data->first_only)
               .keep()
               .help("Use user provided first action or user provided pdf or uniform random"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (data->num_actions == 0) return nullptr;
  if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
  { THROW("error: min and max values must be supplied with cb_continuous"); }
  if (data->bandwidth <= -1) { THROW("error: Bandwidth must be positive"); }
  // Translate user provided bandwidth which is in terms of continuous action range
  // to the internal tree bandwidth which is in terms of #actions
  std::cout << data->max_value << " " << data->min_value << " " << data->num_actions << std::endl;
  float leaf_width = (data->max_value - data->min_value) / (data->num_actions);  // aka unit range
  float half_leaf_width = leaf_width / 2.f;

  uint32_t tree_bandwidth = 0;

  if (data->bandwidth <= half_leaf_width) { tree_bandwidth = 0; }
  else if (std::fmod((data->bandwidth), leaf_width) == 0)
  {
    tree_bandwidth = ((data->bandwidth) / leaf_width);
  }
  else
  {
    tree_bandwidth = ((data->bandwidth) / leaf_width) + 1;
  }

  options.replace("tree_bandwidth", std::to_string(tree_bandwidth));

  std::cout << "--------------------------------------" << std::endl;
  std::cout << "bandwidth provided: " << data->bandwidth << ", tree bandwidth: " << tree_bandwidth << std::endl;
  std::cout << leaf_width << "/" << half_leaf_width << std::endl;
  std::cout << "--------------------------------------" << std::endl;

  auto p_base = as_singleline(setup_base(options, all));
  data->_p_base = p_base;

  learner<pmf_to_pdf::reduction, example>& l = init_learner(data, p_base, learn, predict, 1, prediction_type_t::pdf);

  all.delete_prediction = continuous_actions::delete_probability_density_function;

  return make_base(l);
}

}  // namespace pmf_to_pdf
}  // namespace VW
