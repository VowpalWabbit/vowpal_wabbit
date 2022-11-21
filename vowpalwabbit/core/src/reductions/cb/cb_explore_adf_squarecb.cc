// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_squarecb.h"

#include "vw/config/options.h"
#include "vw/core/action_score.h"
#include "vw/core/cb.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_parser.h"
#include "vw/core/parser.h"
#include "vw/core/rand48.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/core/version.h"
#include "vw/core/vw_versions.h"
#include "vw/explore/explore.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>
using namespace VW::cb_explore_adf;

/*
This file implements the SquareCB algorithm/reduction (Foster and Rakhlin (2020), https://arxiv.org/abs/2002.04926),
with the VW learner as the base algorithm.
*/

// All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities
// are the probability with which each action should be replaced to the top of the list.

#define B_SEARCH_MAX_ITER 20

using namespace VW::LEARNER;

namespace
{
class cb_explore_adf_squarecb
{
public:
  cb_explore_adf_squarecb(float gamma_scale, float gamma_exponent, bool elim, float c0, float min_cb_cost,
      float max_cb_cost, VW::version_struct model_file_version, float epsilon);
  ~cb_explore_adf_squarecb() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(multi_learner& base, VW::multi_ex& examples);
  void learn(multi_learner& base, VW::multi_ex& examples);
  void save_load(io_buf& io, bool read, bool text);

private:
  size_t _counter;
  float _gamma_scale;     // Scale factor for SquareCB reediness parameter $\gamma$.
  float _gamma_exponent;  // Exponent on $t$ for SquareCB reediness parameter $\gamma$.

  // Parameters and data structures for RegCB action set computation
  bool _elim;
  float _c0;
  float _min_cb_cost;
  float _max_cb_cost;
  float _epsilon;

  std::vector<float> _min_costs;
  std::vector<float> _max_costs;

  VW::version_struct _model_file_version;

  // for backing up cb example data when computing sensitivities
  std::vector<VW::action_scores> _ex_as;
  std::vector<std::vector<CB::cb_class>> _ex_costs;
  void get_cost_ranges(float delta, multi_learner& base, VW::multi_ex& examples, bool min_only);
  float binary_search(float fhat, float delta, float sens, float tol = 1e-6);
};

cb_explore_adf_squarecb::cb_explore_adf_squarecb(float gamma_scale, float gamma_exponent, bool elim, float c0,
    float min_cb_cost, float max_cb_cost, VW::version_struct model_file_version, float epsilon)
    : _counter(0)
    , _gamma_scale(gamma_scale)
    , _gamma_exponent(gamma_exponent)
    , _elim(elim)
    , _c0(c0)
    , _min_cb_cost(min_cb_cost)
    , _max_cb_cost(max_cb_cost)
    , _epsilon(epsilon)
    , _model_file_version(model_file_version)
{
}

// TODO: same as cs_active.cc and cb_explore_adf_regcb.cc, move to shared place
float cb_explore_adf_squarecb::binary_search(float fhat, float delta, float sens, float tol)
{
  /*
     Binary search to find the largest weight w such that w*(fhat^2 - (fhat - w*sens)^2) \leq delta.
     Implements binary search procedure described at the end of Section 7.1 in https://arxiv.org/pdf/1703.01014.pdf.
  */

  // We are always guaranteed that the solution to the problem above lies in (0, maxw), as long as fhat \geq 0.
  const float maxw = (std::min)(fhat / sens, FLT_MAX);

  // If the objective value for maxw satisfies the delta constraint, we can just take this and skip the binary search.
  if (maxw * fhat * fhat <= delta) { return maxw; }

  // Upper and lower bounds on w for binary search.
  float l = 0;
  float u = maxw;
  // Binary search variable.
  float w;
  // Value for w.
  float v;

  // Standard binary search given the objective described above.
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

// TODO: Same as cb_explore_adf_regcb.cc
void cb_explore_adf_squarecb::get_cost_ranges(float delta, multi_learner& base, VW::multi_ex& examples, bool min_only)
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

void cb_explore_adf_squarecb::predict(multi_learner& base, VW::multi_ex& examples)
{
  multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

  VW::v_array<VW::action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = static_cast<uint32_t>(preds.size());

  // The actual parameter $\gamma$ used in the SquareCB.
  const float gamma = _gamma_scale * static_cast<float>(std::pow(_counter, _gamma_exponent));

  // RegCB action set parameters
  const float max_range = _max_cb_cost - _min_cb_cost;
  // threshold on empirical loss difference
  const float delta =
      _c0 * std::log(static_cast<float>(num_actions * _counter)) * static_cast<float>(std::pow(max_range, 2));

  // SquareCB Exploration
  if (!_elim)  // Vanilla variant (perform SquareCB exploration over all actions)
  {
    size_t a_min = 0;
    float min_cost = preds[0].score;
    // Compute highest-scoring action
    for (size_t a = 0; a < num_actions; ++a)
    {
      if (preds[a].score < min_cost)
      {
        a_min = a;
        min_cost = preds[a].score;
      }
    }
    // Compute probabilities using SquareCB rule.
    float total_weight = 0;
    float pa = 0;
    for (size_t a = 0; a < num_actions; ++a)
    {
      if (a == a_min) { continue; }
      pa = 1.f / (num_actions + gamma * (preds[a].score - min_cost));
      preds[a].score = pa;
      total_weight += pa;
    }
    preds[a_min].score = 1.f - total_weight;

    exploration::enforce_minimum_probability(_epsilon, true, begin_scores(preds), end_scores(preds));
  }
  else  // elimination variant
  {
    get_cost_ranges(delta, base, examples, /*min_only=*/false);

    float min_max_cost = FLT_MAX;
    for (size_t a = 0; a < num_actions; ++a)
    {
      if (_max_costs[a] < min_max_cost) { min_max_cost = _max_costs[a]; }
    }

    size_t a_min = 0;
    size_t num_surviving_actions = 0;
    float min_cost = FLT_MAX;
    // Compute plausible / surviving actions.
    for (size_t a = 0; a < num_actions; ++a)
    {
      if (preds[a].score < min_cost && _min_costs[preds[a].action] <= min_max_cost)
      {
        a_min = a;
        min_cost = preds[a].score;
        num_surviving_actions += 1;
      }
    }
    float total_weight = 0;
    float pa = 0;
    // Compute probabilities for surviving actions using SquareCB rule.
    for (size_t a = 0; a < num_actions; ++a)
    {
      if (_min_costs[preds[a].action] > min_max_cost) { preds[a].score = 0; }
      else
      {
        if (a == a_min) { continue; }
        pa = 1.f / (num_surviving_actions + gamma * (preds[a].score - min_cost));
        preds[a].score = pa;
        total_weight += pa;
      }
    }
    preds[a_min].score = 1.f - total_weight;
  }
}

void cb_explore_adf_squarecb::learn(multi_learner& base, VW::multi_ex& examples)
{
  VW::v_array<VW::action_score> preds = std::move(examples[0]->pred.a_s);
  for (size_t i = 0; i < examples.size() - 1; ++i)
  {
    CB::label& ld = examples[i]->l.cb;
    if (ld.costs.size() == 1)
    {
      ld.costs[0].probability = 1.f;  // no importance weighting
    }
  }

  multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
  ++_counter;
  examples[0]->pred.a_s = std::move(preds);
}

void cb_explore_adf_squarecb::save_load(io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (!read || _model_file_version >= VW::version_definitions::VERSION_FILE_WITH_SQUARE_CB_SAVE_RESUME)
  {
    std::stringstream msg;
    if (!read) { msg << "cb squarecb adf storing example counter:  = " << _counter << "\n"; }
    bin_text_read_write_fixed_validated(io, reinterpret_cast<char*>(&_counter), sizeof(_counter), read, msg, text);
  }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::cb_explore_adf_squarecb_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool squarecb = false;
  std::string type_string = "mtr";

  // Basic SquareCB parameters
  float gamma_scale = 1.;
  float gamma_exponent = 0.;

  // Perform SquareCB exploration over RegCB-style disagreement sets
  bool elim = false;
  float c0 = 0.;
  float min_cb_cost = 0.;
  float max_cb_cost = 0.;
  float epsilon = 0.f;

  config::option_group_definition new_options("[Reduction] Contextual Bandit Exploration with ADF (SquareCB)");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("squarecb", squarecb).keep().necessary().help("SquareCB exploration"))
      .add(make_option("gamma_scale", gamma_scale)
               .keep()
               .default_value(10.f)
               .help("Sets SquareCB greediness parameter to gamma=[gamma_scale]*[num examples]^1/2"))
      .add(make_option("gamma_exponent", gamma_exponent)
               .keep()
               .default_value(.5f)
               .help("Exponent on [num examples] in SquareCB greediness parameter gamma"))
      .add(make_option("elim", elim)
               .keep()
               .help("Only perform SquareCB exploration over plausible actions (computed via RegCB strategy)"))
      .add(make_option("mellowness", c0)
               .keep()
               .default_value(0.001f)
               .help("Mellowness parameter c_0 for computing plausible action set. Only used with --elim"))
      .add(make_option("cb_min_cost", min_cb_cost)
               .keep()
               .default_value(0.f)
               .help("Lower bound on cost. Only used with --elim"))
      .add(make_option("cb_max_cost", max_cb_cost)
               .keep()
               .default_value(1.f)
               .help("Upper bound on cost. Only used with --elim"))
      .add(make_option("cb_type", type_string)
               .keep()
               .default_value("mtr")
               .one_of({"mtr"})
               .help("Contextual bandit method to use. SquareCB only supports supervised regression (mtr)"))
      // Note: --epsilon for squarecb is semantically different than adding a uniform distribution on all actions
      // as done in epsilon greedy.
      .add(make_option("epsilon", epsilon)
               .keep()
               .default_value(0.f)
               .allow_override()
               .help("The minimum probability of an action is this value divided by the number of actions."));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Ensure serialization of this option in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  // Set explore_type
  size_t problem_multiplier = 1;

  multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  bool with_metrics = options.was_supplied("extra_metrics");

  if (epsilon < 0.0 || epsilon > 1.0) { THROW("The value of epsilon must be in [0,1]"); }

  using explore_type = cb_explore_adf_base<cb_explore_adf_squarecb>;
  auto data = VW::make_unique<explore_type>(
      with_metrics, gamma_scale, gamma_exponent, elim, c0, min_cb_cost, max_cb_cost, all.model_file_ver, epsilon);
  auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_squarecb_setup))
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
                .build(&all.logger);
  return make_base(*l);
}
