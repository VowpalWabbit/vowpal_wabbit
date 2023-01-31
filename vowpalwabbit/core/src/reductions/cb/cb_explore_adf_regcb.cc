// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_regcb.h"

#include "vw/config/options.h"
#include "vw/core/action_score.h"
#include "vw/core/cb.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_parser.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/core/version.h"
#include "vw/core/vw_versions.h"
#include "vw/explore/explore.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>

// All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities
// are the probability with which each action should be replaced to the top of the list.

#define B_SEARCH_MAX_ITER 20

using namespace VW::cb_explore_adf;
using namespace VW::LEARNER;

namespace
{
class cb_explore_adf_regcb
{
public:
  cb_explore_adf_regcb(bool regcbopt, float c0, bool first_only, float min_cb_cost, float max_cb_cost,
      VW::version_struct model_file_version);
  ~cb_explore_adf_regcb() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(learner& base, VW::multi_ex& examples) { predict_impl(base, examples); }
  void learn(learner& base, VW::multi_ex& examples) { learn_impl(base, examples); }
  void save_load(VW::io_buf& io, bool read, bool text);

private:
  void predict_impl(learner& base, VW::multi_ex& examples);
  void learn_impl(learner& base, VW::multi_ex& examples);

  void get_cost_ranges(float delta, learner& base, VW::multi_ex& examples, bool min_only);
  float binary_search(float fhat, float delta, float sens, float tol = 1e-6);

private:
  size_t _counter;
  bool _regcbopt;  // use optimistic variant of RegCB
  float _c0;       // mellowness parameter for RegCB
  bool _first_only;
  float _min_cb_cost;
  float _max_cb_cost;

  std::vector<float> _min_costs;
  std::vector<float> _max_costs;

  VW::version_struct _model_file_version;

  // for backing up cb example data when computing sensitivities
  std::vector<VW::action_scores> _ex_as;
  std::vector<std::vector<VW::cb_class>> _ex_costs;
};

cb_explore_adf_regcb::cb_explore_adf_regcb(bool regcbopt, float c0, bool first_only, float min_cb_cost,
    float max_cb_cost, VW::version_struct model_file_version)
    : _counter(0)
    , _regcbopt(regcbopt)
    , _c0(c0)
    , _first_only(first_only)
    , _min_cb_cost(min_cb_cost)
    , _max_cb_cost(max_cb_cost)
    , _model_file_version(model_file_version)
{
}

// TODO: same as cs_active.cc, move to shared place
float cb_explore_adf_regcb::binary_search(float fhat, float delta, float sens, float tol)
{
  const float maxw = (std::min)(fhat / sens, FLT_MAX);

  if (maxw * fhat * fhat <= delta) { return maxw; }

  float l = 0;
  float u = maxw;
  float w, v;

  for (int iter = 0; iter < B_SEARCH_MAX_ITER; iter++)
  {
    w = (u + l) / 2.f;
    v = w * (fhat * fhat - (fhat - sens * w) * (fhat - sens * w)) - delta;
    if (v > 0) { u = w; }
    else { l = w; }
    if (std::fabs(v) <= tol || u - l <= tol) { break; }
  }

  return l;
}

void cb_explore_adf_regcb::get_cost_ranges(float delta, learner& base, VW::multi_ex& examples, bool min_only)
{
  const size_t num_actions = examples[0]->pred.a_s.size();
  _min_costs.resize(num_actions);
  _max_costs.resize(num_actions);

  _ex_as.clear();
  _ex_costs.clear();

  // backup cb example data
  for (const auto& ex : examples)
  {
    _ex_as.push_back(ex->pred.a_s);
    _ex_costs.push_back(ex->l.cb.costs);
  }

  // set regressor predictions
  for (const auto& as : _ex_as[0]) { examples[as.action]->pred.scalar = as.score; }

  const float cmin = _min_cb_cost;
  const float cmax = _max_cb_cost;

  for (size_t a = 0; a < num_actions; ++a)
  {
    auto* ec = examples[a];
    ec->l.simple.label = cmin - 1;
    float sens = base.sensitivity(*ec);
    float w = 0;  // importance weight

    if (ec->pred.scalar < cmin || std::isnan(sens) || std::isinf(sens)) { _min_costs[a] = cmin; }
    else
    {
      w = binary_search(ec->pred.scalar - cmin + 1, delta, sens);
      _min_costs[a] = (std::max)(ec->pred.scalar - sens * w, cmin);
      if (_min_costs[a] > cmax) { _min_costs[a] = cmax; }
    }

    if (!min_only)
    {
      ec->l.simple.label = cmax + 1;
      sens = base.sensitivity(*ec);
      if (ec->pred.scalar > cmax || std::isnan(sens) || std::isinf(sens)) { _max_costs[a] = cmax; }
      else
      {
        w = binary_search(cmax + 1 - ec->pred.scalar, delta, sens);
        _max_costs[a] = (std::min)(ec->pred.scalar + sens * w, cmax);
        if (_max_costs[a] < cmin) { _max_costs[a] = cmin; }
      }
    }
  }

  // reset cb example data
  for (size_t i = 0; i < examples.size(); ++i)
  {
    examples[i]->pred.a_s = _ex_as[i];
    examples[i]->l.cb.costs = _ex_costs[i];
  }
}

void cb_explore_adf_regcb::predict_impl(learner& base, VW::multi_ex& examples)
{
  multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);
  VW::v_array<VW::action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = static_cast<uint32_t>(preds.size());

  const float max_range = _max_cb_cost - _min_cb_cost;
  // threshold on empirical loss difference
  const float delta =
      _c0 * std::log(static_cast<float>(num_actions * _counter)) * static_cast<float>(std::pow(max_range, 2));
  get_cost_ranges(delta, base, examples, /*min_only=*/_regcbopt);

  if (_regcbopt)  // optimistic variant
  {
    float min_cost = FLT_MAX;
    size_t a_opt = 0;  // optimistic action
    for (size_t a = 0; a < num_actions; ++a)
    {
      if (_min_costs[a] < min_cost)
      {
        min_cost = _min_costs[a];
        a_opt = a;
      }
    }
    for (size_t i = 0; i < preds.size(); ++i)
    {
      if (preds[i].action == a_opt || (!_first_only && _min_costs[preds[i].action] == min_cost)) { preds[i].score = 1; }
      else { preds[i].score = 0; }
    }
  }
  else  // elimination variant
  {
    float min_max_cost = FLT_MAX;
    for (size_t a = 0; a < num_actions; ++a)
    {
      if (_max_costs[a] < min_max_cost) { min_max_cost = _max_costs[a]; }
    }
    for (size_t i = 0; i < preds.size(); ++i)
    {
      if (_min_costs[preds[i].action] <= min_max_cost) { preds[i].score = 1; }
      else { preds[i].score = 0; }
      // explore uniformly on support
      VW::explore::enforce_minimum_probability(
          1.0, /*update_zero_elements=*/false, begin_scores(preds), end_scores(preds));
    }
  }
}

void cb_explore_adf_regcb::learn_impl(learner& base, VW::multi_ex& examples)
{
  VW::v_array<VW::action_score> preds = std::move(examples[0]->pred.a_s);
  for (size_t i = 0; i < examples.size() - 1; ++i)
  {
    VW::cb_label& ld = examples[i]->l.cb;
    if (ld.costs.size() == 1)
    {
      ld.costs[0].probability = 1.f;  // no importance weighting
    }
  }

  multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
  ++_counter;
  examples[0]->pred.a_s = std::move(preds);
}

void cb_explore_adf_regcb::save_load(VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (!read || _model_file_version >= VW::version_definitions::VERSION_FILE_WITH_REG_CB_SAVE_RESUME)
  {
    std::stringstream msg;
    if (!read) { msg << "cb squarecb adf storing example counter:  = " << _counter << "\n"; }
    VW::details::bin_text_read_write_fixed_validated(
        io, reinterpret_cast<char*>(&_counter), sizeof(_counter), read, msg, text);
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_explore_adf_regcb_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool regcb = false;
  const std::string mtr = "mtr";
  std::string type_string(mtr);
  bool regcbopt = false;
  float c0 = 0.;
  bool first_only = false;
  float min_cb_cost = 0.;
  float max_cb_cost = 0.;
  config::option_group_definition new_options("[Reduction] Contextual Bandit Exploration with ADF (RegCB)");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .necessary()
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("regcb", regcb).necessary().keep().help("RegCB-elim exploration"))
      .add(make_option("regcbopt", regcbopt).keep().help("RegCB optimistic exploration"))
      .add(make_option("mellowness", c0).keep().default_value(0.1f).help("RegCB mellowness parameter c_0. Default 0.1"))
      .add(make_option("cb_min_cost", min_cb_cost).keep().default_value(0.f).help("Lower bound on cost"))
      .add(make_option("cb_max_cost", max_cb_cost).keep().default_value(1.f).help("Upper bound on cost"))
      .add(make_option("first_only", first_only).keep().help("Only explore the first action in a tie-breaking event"))
      .add(make_option("cb_type", type_string)
               .keep()
               .default_value("mtr")
               .one_of({"mtr"})
               .help("Contextual bandit method to use. RegCB only supports supervised regression (mtr)"));

  auto enabled = options.add_parse_and_check_necessary(new_options);

  if (regcbopt && !regcb)
  {
    all.logger.err_warn(
        "RegCB used to be able to be enabled with either --regcb or --regcbopt. Enabling with --regcbopt only is now "
        "deprecated. Please add --regcb to your command line in addition to --regcbopt.");
    enabled = true;
  }
  if (!enabled) { return nullptr; }

  // Ensure serialization of cb_type in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  // Set explore_type
  size_t problem_multiplier = 1;

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::cb_label_parser_global;

  using explore_type = cb_explore_adf_base<cb_explore_adf_regcb>;
  auto data = VW::make_unique<explore_type>(
      all.global_metrics.are_metrics_enabled(), regcbopt, c0, first_only, min_cb_cost, max_cb_cost, all.model_file_ver);
  auto l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_regcb_setup))
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_params_per_weight(problem_multiplier)
               .set_output_example_prediction(explore_type::output_example_prediction)
               .set_update_stats(explore_type::update_stats)
               .set_print_update(explore_type::print_update)
               .set_persist_metrics(explore_type::persist_metrics)
               .set_save_load(explore_type::save_load)
               .build();
  return l;
}
