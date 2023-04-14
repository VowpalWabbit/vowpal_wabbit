// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_graph_feedback.h"

#include "vw/common/random.h"
#include "vw/config/options.h"
#include "vw/core/gd_predict.h"
#include "vw/core/global_data.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/label_parser.h"
#include "vw/core/model_utils.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw_math.h"
#include "vw/explore/explore.h"

#include <algorithm>
#define ARMA_DONT_USE_BLAS
#define ARMA_DONT_USE_LAPACK
#include <armadillo>
#include <cmath>
#include <ensmallen.hpp>
#include <functional>
#include <vector>

using namespace VW::cb_explore_adf;

namespace VW
{
namespace cb_explore_adf
{
class cb_explore_adf_graph_feedback
{
public:
  cb_explore_adf_graph_feedback(float gamma_scale, float gamma_exponent, VW::workspace* all)
      : _gamma_scale(gamma_scale), _gamma_exponent(gamma_exponent), _all(all)
  {
  }
  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::learner& base, multi_ex& examples);
  void learn(VW::LEARNER::learner& base, multi_ex& examples);
  void save_load(io_buf& io, bool read, bool text);
  size_t _counter = 0;
  float _gamma_scale;
  float _gamma_exponent;

private:
  VW::workspace* _all;
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::learner& base, multi_ex& examples);
  void update_example_prediction(multi_ex& examples);
};
}  // namespace cb_explore_adf

/**
 * Implementing the constrained optimization from the paper: https://arxiv.org/abs/2302.08631
 * We want to minimize objective: p * fhat + z
 * where p is the resulting probability distribution, fhat is the scores we get from cb_adf and z is a constant
 * The constraints are (very roughly) for each action we want:
 *        (p - ea)**2 / G * p <= fhat_a + z
 * where fhat_a is the cost for that action and ea is the vector of the identity matrix at index a
 *
 * G is a (num_actions x num_actions) graph where each column indicates the probability of that action (corresponding to
 * the column index) revealing the reward for a different action (which row are we talking about)
 *
 * So for example the identity matrix for a 2 x 2 matrix
 * 1 0
 * 0 1
 *
 * means that each action taken will reveal information only for itself
 *
 * and the all 1's matrix
 *
 * 1 1
 * 1 1
 *
 * corresponds to supervised learning where each action taken will reveal infromation for every other action
 *
 *
 * The way to think about gamma, which will increase over time, is that a small gamma means that the final p will
 * "listen" more to the graph and try to give higher probability to the actions that reveal more, even if they have a
 * higher cost. As gamma increases, the final p will "listen" less to the graph and will listen more to the cost, so if
 * an action has a high cost it will have a lower probability of being chosen even if it reveals a lot of information
 * about other actions
 */
class ConstrainedFunctionType
{
  const arma::vec& _fhat;
  const arma::sp_mat& _G;
  const float _gamma;

public:
  ConstrainedFunctionType(const arma::vec& scores, const arma::sp_mat& G, const float gamma)
      : _fhat(scores), _G(G), _gamma(gamma)
  {
  }

  // Return the objective function f(x) for the given x.
  double Evaluate(const arma::mat& x) const
  {
    arma::mat p(x.n_rows - 1, 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p[i] = x[i]; }

    float z = x[x.n_rows - 1];

    return (arma::dot(p, _fhat) + z);
  }

  // Compute the gradient of f(x) for the given x and store the result in g.
  void Gradient(const arma::mat&, arma::mat& g) const
  {
    g.set_size(_fhat.n_rows + 1, 1);
    for (size_t i = 0; i < _fhat.n_rows; ++i) { g[i] = _fhat(i); }
    g[_fhat.n_rows] = 1.f;
  }

  // Get the number of constraints on the objective function.
  size_t NumConstraints() const { return _fhat.size() + 4; }

  // Evaluate constraint i at the parameters x.  If the constraint is
  // unsatisfied, a value greater than 0 should be returned.  If the constraint
  // is satisfied, 0 should be returned.  The optimizer will add this value to
  // its overall objective that it is trying to minimize.
  double EvaluateConstraint(const size_t i, const arma::mat& x) const
  {
    arma::vec p(x.n_rows - 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p(i) = x[i]; }

    float z = x[x.n_rows - 1];

    if (i < _fhat.size())
    {
      arma::vec eyea = arma::zeros<arma::vec>(p.n_rows);
      eyea(i) = 1.f;

      auto fhata = _fhat(i);

      double sum = 0.f;
      for (size_t index = 0; index < p.n_rows; index++)
      {
        arma::vec Ga(_G.row(index).n_cols);
        for (size_t j = 0; j < _G.row(index).n_cols; j++) { Ga(j) = _G.row(index)(j); }

        auto Ga_times_p = arma::dot(Ga, p);

        float denominator = Ga_times_p;
        auto nominator = (eyea(index) - p(index)) * (eyea(index) - p(index));
        sum += (nominator / denominator);
      }
      if (sum <= (fhata + z)) { return 0.f; }
      else { return sum - (fhata + z); }
    }
    else if (i == _fhat.size())
    {
      if (arma::all(p >= 0.f)) { return 0.f; }
      double neg_sum = 0.;
      for (size_t i = 0; i < p.n_rows; i++)
      {
        if (p(i) < 0) { neg_sum += p(i); }
      }
      // negative probabilities are really really bad
      return -100.f * _gamma * neg_sum;
    }
    else if (i == _fhat.size() + 1)
    {
      if (arma::sum(p) <= 1.f) { return 0.f; }
      return arma::sum(p) - 1.f;
    }
    else if (i == _fhat.size() + 2)
    {
      if (arma::sum(p) >= 1.f) { return 0.f; }
      return 1.f - arma::sum(p);
    }
    else if (i == _fhat.size() + 3)
    {
      if ((z / _gamma) > 0.f) { return 0.f; }
      return 0.f - (z / _gamma);
    }
    return 0.;
  }

  // Evaluate the gradient of constraint i at the parameters x, storing the
  // result in the given matrix g.  If the constraint is not satisfied, the
  // gradient should be set in such a way that the gradient points in the
  // direction where the constraint would be satisfied.
  void GradientConstraint(const size_t i, const arma::mat& x, arma::mat& g) const
  {
    arma::vec p(x.n_rows - 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p(i) = x[i]; }

    float z = x[x.n_rows - 1];

    double constraint = EvaluateConstraint(i, x);

    if (i < _fhat.size())
    {
      g.set_size(_fhat.n_rows + 1, 1);
      g.zeros();

      g[_fhat.size()] = 1.f;

      arma::vec eyea = arma::zeros<arma::vec>(p.n_rows);
      eyea(i) = 1.f;

      for (size_t coord_i = 0; coord_i < _fhat.size(); coord_i++)
      {
        double sum = 0.f;
        for (size_t index = 0; index < p.n_rows; index++)
        {
          arma::vec Ga(_G.row(index).n_cols);
          for (size_t j = 0; j < _G.row(index).n_cols; j++) { Ga(j) = _G.row(index)(j); }
          auto Ga_times_p = arma::dot(Ga, p);

          if (index == coord_i)
          {
            float denominator = Ga_times_p * Ga_times_p;

            auto b = _G.row(index)(index);
            auto c = Ga_times_p - b * p(index);
            auto nominator = -1.f * ((eyea(index) - p(index)) * (eyea(index) * b + b * p(index) + 2.f * c));

            sum += (nominator / denominator);
          }
          else
          {
            auto a = (eyea(index) - p(index)) * (eyea(index) - p(index));
            auto b = _G.row(index)(coord_i);

            auto nominator = -1.f * ((a * b));
            auto denominator = Ga_times_p * Ga_times_p;
            sum += nominator / denominator;
          }
        }

        g[coord_i] = sum;
      }

      if (constraint == 0.f)
      {
        g = -1.f * g;
        // restore original point not concerned with the constraint
        g[_fhat.size()] = 1.f;
      }
    }
    else if (i == _fhat.size())
    {
      // all positives
      g.set_size(_fhat.n_rows + 1, 1);
      g.ones();

      for (size_t i = 0; i < p.n_rows; ++i)
      {
        if (p(i) < 0.f)
        {
          for (size_t i = 0; i < _fhat.size(); i++) { g[i] = -1.f; }
        }
      }

      g[_fhat.size()] = 0.f;
    }
    else if (i == _fhat.size() + 1)
    {
      // sum
      g.set_size(_fhat.n_rows + 1, 1);
      g.ones();

      if (constraint == 0.f) { g = -1.f * g; }

      g[_fhat.size()] = 0.f;
    }
    else if (i == _fhat.size() + 2)
    {
      // sum
      g.set_size(_fhat.n_rows + 1, 1);
      g.ones();

      if (constraint != 0.f) { g = -1.f * g; }

      g[_fhat.size()] = 0.f;
    }
    else if (i == _fhat.size() + 3)
    {
      g.set_size(_fhat.n_rows + 1, 1);
      g.ones();

      if ((z / _gamma) < 0.f) { g[_fhat.size()] = -1.f; }
    }
  }
};

bool valid_graph(const std::vector<VW::cb_graph_feedback::triplet>& triplets)
{
  // return false if all triplet vals are zero
  for (auto& triplet : triplets)
  {
    if (triplet.val != 0.f) { return true; }
  }
  return false;
}

std::pair<arma::mat, arma::vec> set_initial_coordinates(const arma::vec& fhat, float gamma)
{
  // find fhat min
  auto min_fhat = fhat.min();
  arma::vec gammafhat = gamma * (fhat - min_fhat);

  // initial p can be uniform random
  arma::mat coordinates(gammafhat.size() + 1, 1);
  for (size_t i = 0; i < gammafhat.size(); i++) { coordinates[i] = 1.f / gammafhat.size(); }

  // initial z can be 1
  // but also be nice if all fhat's are zero
  float z = gamma * (1 - (min_fhat == 0 ? 1.f / fhat.size() : min_fhat));

  coordinates[gammafhat.size()] = z;

  return {coordinates, gammafhat};
}

arma::vec get_probs_from_coordinates(arma::mat& coordinates, const arma::vec& fhat, VW::workspace& all)
{
  // constraints are enforcers but they can be broken, so we need to check that probs are positive and sum to 1

  // we also have to check for nan's because some starting points combined with some gammas might make the constraint
  // optimization go off the charts; it should be rare but we need to guard against it

  size_t num_actions = coordinates.n_rows - 1;
  auto count_zeros = 0;
  bool there_is_a_one = false;
  bool there_is_a_nan = false;

  for (size_t i = 0; i < num_actions; i++)
  {
    if (VW::math::are_same(static_cast<float>(coordinates[i]), 0.f) || coordinates[i] < 0.f)
    {
      coordinates[i] = 0.f;
      count_zeros++;
    }
    if (coordinates[i] > 1.f)
    {
      coordinates[i] = 1.f;
      there_is_a_one = true;
    }
    if (std::isnan(coordinates[i])) { there_is_a_nan = true; }
  }

  if (there_is_a_nan)
  {
    for (size_t i = 0; i < num_actions; i++) { coordinates[i] = 1.f - fhat(i); }
  }

  if (there_is_a_one)
  {
    for (size_t i = 0; i < num_actions; i++)
    {
      if (coordinates[i] != 1.f) { coordinates[i] = 0.f; }
    }
  }

  float p_sum = 0;
  for (size_t i = 0; i < num_actions; i++) { p_sum += coordinates[i]; }

  if (!VW::math::are_same(p_sum, 1.f))
  {
    float rest = 1.f - p_sum;
    float rest_each = rest / (num_actions - count_zeros);
    for (size_t i = 0; i < num_actions; i++)
    {
      if (coordinates[i] == 0.f) { continue; }
      else { coordinates[i] = coordinates[i] + rest_each; }
    }
  }

  float sum = 0;
  arma::vec probs(num_actions);
  for (size_t i = 0; i < probs.n_rows; ++i)
  {
    probs(i) = coordinates[i];
    sum += probs(i);
  }

  if (!VW::math::are_same(sum, 1.f))
  {
    // leaving this here just in case this happens for some reason that we did not think to check for
    all.logger.warn("Probabilities do not sum to 1, they sum to: {}", sum);
  }

  return probs;
}

arma::sp_mat get_graph(const VW::cb_graph_feedback::reduction_features& graph_reduction_features, size_t num_actions)
{
  arma::sp_mat G(num_actions, num_actions);

  if (valid_graph(graph_reduction_features.triplets))
  {
    arma::umat locations(2, graph_reduction_features.triplets.size());

    arma::vec values(graph_reduction_features.triplets.size());

    for (size_t i = 0; i < graph_reduction_features.triplets.size(); i++)
    {
      const auto& triplet = graph_reduction_features.triplets[i];
      locations(0, i) = triplet.row;
      locations(1, i) = triplet.col;
      values(i) = triplet.val;
    }

    G = arma::sp_mat(true, locations, values, num_actions, num_actions);
  }
  else { G = arma::speye<arma::sp_mat>(num_actions, num_actions); }
  return G;
}

void cb_explore_adf_graph_feedback::update_example_prediction(multi_ex& examples)
{
  auto& a_s = examples[0]->pred.a_s;
  size_t num_actions = a_s.size();
  arma::vec fhat(a_s.size());

  for (auto& as : a_s) { fhat(as.action) = as.score; }
  const float gamma = _gamma_scale * static_cast<float>(std::pow(_counter, _gamma_exponent));

  auto coord_gammafhat = set_initial_coordinates(fhat, gamma);
  arma::mat coordinates = std::get<0>(coord_gammafhat);
  arma::vec gammafhat = std::get<1>(coord_gammafhat);

  auto& graph_reduction_features =
      examples[0]->ex_reduction_features.template get<VW::cb_graph_feedback::reduction_features>();
  arma::sp_mat G = get_graph(graph_reduction_features, num_actions);

  ConstrainedFunctionType f(gammafhat, G, gamma);

  ens::AugLagrangian optimizer;
  optimizer.Optimize(f, coordinates);

  // TODO json graph input

  arma::vec probs = get_probs_from_coordinates(coordinates, fhat, *_all);

  // set the new probabilities in the example
  for (auto& as : a_s) { as.score = probs(as.action); }
  std::sort(
      a_s.begin(), a_s.end(), [](const VW::action_score& a, const VW::action_score& b) { return a.score > b.score; });
}

template <bool is_learn>
void cb_explore_adf_graph_feedback::predict_or_learn_impl(VW::LEARNER::learner& base, multi_ex& examples)
{
  if (is_learn)
  {
    _counter++;
    base.learn(examples);
    if (base.learn_returns_prediction) { update_example_prediction(examples); }
  }
  else
  {
    base.predict(examples);
    update_example_prediction(examples);
  }
}

void cb_explore_adf_graph_feedback::predict(VW::LEARNER::learner& base, multi_ex& examples)
{
  predict_or_learn_impl<false>(base, examples);
}

void cb_explore_adf_graph_feedback::learn(VW::LEARNER::learner& base, multi_ex& examples)
{
  predict_or_learn_impl<true>(base, examples);
}

void cb_explore_adf_graph_feedback::save_load(VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (!read)
  {
    std::stringstream msg;
    if (!read) { msg << "cb adf with graph feedback storing example counter:  = " << _counter << "\n"; }
    VW::details::bin_text_read_write_fixed_validated(
        io, reinterpret_cast<char*>(&_counter), sizeof(_counter), read, msg, text);
  }
}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_explore_adf_graph_feedback_setup(
    VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool graph_feedback = false;
  float gamma_scale = 1.;
  float gamma_exponent = 0.;

  config::option_group_definition new_options(
      "[Reduction] Experimental: Contextual Bandit Exploration with ADF with graph feedback");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("gamma_scale", gamma_scale)
               .keep()
               .default_value(1.f)
               .help("Sets CB with graph feedback gamma parameter to gamma=[gamma_scale]*[num examples]^1/2"))
      .add(make_option("gamma_exponent", gamma_exponent)
               .keep()
               .default_value(.5f)
               .help("Exponent on [num examples] in CB with graph feedback parameter gamma"))
      .add(make_option("graph_feedback", graph_feedback).necessary().keep().help("Graph feedback pdf").experimental());

  auto enabled = options.add_parse_and_check_necessary(new_options);
  if (!enabled) { return nullptr; }

  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.parser_runtime.example_parser->lbl_parser = VW::cb_label_parser_global;

  using explore_type = cb_explore_adf_base<cb_explore_adf_graph_feedback>;

  size_t problem_multiplier = 1;
  bool with_metrics = options.was_supplied("extra_metrics");

  auto data = VW::make_unique<explore_type>(with_metrics, gamma_scale, gamma_exponent, &all);

  auto l = VW::LEARNER::make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(VW::reductions::cb_explore_adf_graph_feedback_setup))
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_feature_width(problem_multiplier)
               .set_output_example_prediction(explore_type::output_example_prediction)
               .set_update_stats(explore_type::update_stats)
               .set_print_update(explore_type::print_update)
               .set_persist_metrics(explore_type::persist_metrics)
               .set_save_load(explore_type::save_load)
               .set_learn_returns_prediction(base->learn_returns_prediction)
               .build();
  return l;
}
}  // namespace VW