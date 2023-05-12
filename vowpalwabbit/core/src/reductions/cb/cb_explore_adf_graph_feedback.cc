// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_graph_feedback.h"

#include "vw/common/random.h"
#include "vw/common/vw_exception.h"
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
#include "vw/core/scope_exit.h"
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

// potential bug in the ensmallen library
// below method template specialization to be removed once/if that is resolved and the library is updated
// https://github.com/mlpack/ensmallen/issues/365
namespace ens
{
class L_BFGS;

template <>
double L_BFGS::ChooseScalingFactor(const size_t iterationNum, const arma::Mat<double>& gradient,
    const arma::Cube<double>& s, const arma::Cube<double>& y)
{
  typedef typename arma::Cube<double>::elem_type CubeElemType;

  double scalingFactor;
  if (iterationNum > 0)
  {
    int previousPos = (iterationNum - 1) % numBasis;
    // Get s and y matrices once instead of multiple times.
    const arma::Mat<CubeElemType>& sMat = s.slice(previousPos);
    const arma::Mat<CubeElemType>& yMat = y.slice(previousPos);

    scalingFactor = dot(sMat, yMat) / std::max(1e-12, dot(yMat, yMat));
  }
  else { scalingFactor = 1.0 / sqrt(dot(gradient, gradient)); }

  return scalingFactor;
}
}  // namespace ens

namespace VW
{
namespace cb_explore_adf
{
constexpr double ALMOST_ZERO = 1e-6;
constexpr double EPSILON = 1e-3;
constexpr double GAMMA_CUTOFF = 1.0;

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
  void update_example_prediction(multi_ex& examples, const arma::sp_mat& G);
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
  const double _gamma;

public:
  ConstrainedFunctionType(const arma::vec& scores, const arma::sp_mat& G, const double gamma)
      : _fhat(scores), _G(G), _gamma(gamma)
  {
  }

  // Return the objective function f(x) for the given x.
  double Evaluate(const arma::mat& x)
  {
    arma::mat p(x.n_rows - 1, 1);
    for (size_t i = 0; i < x.n_rows - 1; ++i) { p[i] = x[i]; }

    auto z = x[x.n_rows - 1];

    return (arma::dot(_fhat, p) + z);
  }

  // Compute the gradient of f(x) for the given x and store the result in g.
  void Gradient(const arma::mat& x, arma::mat& g)
  {
    g.set_size(x.n_rows, 1);
    g.zeros();
    for (size_t i = 0; i < x.n_rows - 1; ++i) { g[i] = _fhat[i]; }
    g[x.n_rows - 1] = 1.0;
  }

  void NumericalGradient(const arma::mat& x, arma::mat& g)
  {
    g.set_size(_fhat.n_rows + 1, 1);
    g.zeros();
    for (size_t i = 0; i < _fhat.n_rows; ++i)
    {
      arma::mat x_i_plus = x;
      x_i_plus(i) += EPSILON;
      arma::mat x_i_minus = x;
      x_i_minus(i) -= EPSILON;
      g(i) = (Evaluate(x_i_plus) - Evaluate(x_i_minus)) / (2.0 * EPSILON);
    }
  }

  // Get the number of constraints on the objective function.
  size_t NumConstraints() const { return 3 * _fhat.size() + 2; }

  // Evaluate constraint i at the parameters x.  If the constraint is
  // unsatisfied, a value greater than 0 should be returned.  If the constraint
  // is satisfied, 0 should be returned.  The optimizer will add this value to
  // its overall objective that it is trying to minimize.
  double EvaluateConstraint(const size_t i, const arma::mat& x)
  {
    arma::vec p(x.n_rows - 1);
    for (size_t i = 0; i < x.n_rows - 1; ++i) { p[i] = x[i]; }

    double z = x[x.n_rows - 1];

    if (i < _fhat.size())
    {
      arma::vec eyea = arma::zeros<arma::vec>(p.n_rows);
      eyea(i) = 1;

      double fhata = _fhat(i);

      double sum = 0;
      for (size_t index = 0; index < p.n_rows; index++)
      {
        double Ga_times_p = std::max(arma::dot(_G.row(index), p.t()), ALMOST_ZERO);

        double g_x = Ga_times_p;
        double f_x = (eyea(index) - p(index)) * (eyea(index) - p(index));
        sum += (f_x / g_x);
      }

      double lhs = _gamma < GAMMA_CUTOFF ? sum : sum / _gamma;
      double rhs = _gamma < GAMMA_CUTOFF ? _gamma * (fhata + z) : (fhata + z);
      if (lhs <= rhs) { return 0.0; }
      return lhs - rhs;
    }
    else if (i == _fhat.size())
    {
      if (arma::sum(p) >= (1.0 - 1e-3)) { return 0.0; }
      return (1.0 - 1e-3) - arma::sum(p);
    }
    else if (i == _fhat.size() + 1)
    {
      if (arma::sum(p) <= (1.0 + 1e-3)) { return 0.0; }
      return arma::sum(p) - (1.0 + 1e-3);
    }
    else if (i > _fhat.size() + 1 && i < 2 * _fhat.size() + 2)
    {
      size_t index = i - _fhat.size() - 2;

      double Gpa = arma::dot(_G.row(index), p.t());

      if (Gpa >= ALMOST_ZERO) { return 0; }
      return ALMOST_ZERO - Gpa;
    }
    else if (i >= 2 * _fhat.size() + 2)
    {
      size_t index = i - 2 * _fhat.size() - 2;
      if (p[index] >= 0.0) { return 0.0; }
      return -1.0 * p[index];
    }
    return 0.0;
  }

  void NumericalGradientConstraint(const size_t i, const arma::mat& x, arma::mat& g)
  {
    g.set_size(_fhat.n_rows + 1, 1);
    g.zeros();

    for (size_t j = 0; j < _fhat.n_rows; ++j)
    {
      arma::mat x_j_plus = x;
      x_j_plus(j) += EPSILON;
      arma::mat x_j_minus = x;
      x_j_minus(j) -= EPSILON;
      double cp = EvaluateConstraint(i, x_j_plus);
      double cm = EvaluateConstraint(i, x_j_minus);
      g(j) = (cp - cm) / (2.0 * EPSILON);
    }
  }

  // Evaluate the gradient of constraint i at the parameters x, storing the
  // result in the given matrix g.  If the constraint is not satisfied, the
  // gradient should be set in such a way that the gradient points in the
  // direction where the constraint would be satisfied.
  void GradientConstraint(const size_t i, const arma::mat& x, arma::mat& g)
  {
    arma::vec p(x.n_rows - 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p(i) = x[i]; }

    g.set_size(_fhat.n_rows + 1, 1);

    if (EvaluateConstraint(i, x) <= 0)
    {
      g.zeros();
      return;
    }

    if (i < _fhat.size())
    {
      g.zeros();

      g[_fhat.size()] = 1.0;

      arma::vec eyea = arma::zeros<arma::vec>(p.n_rows);
      eyea(i) = 1;

      for (size_t coord_i = 0; coord_i < _fhat.size(); coord_i++)
      {
        double sum = 0;
        for (size_t index = 0; index < p.n_rows; index++)
        {
          double Ga_times_p = arma::dot(_G.row(index), p.t());

          double g_x = std::max(Ga_times_p, ALMOST_ZERO);

          double g_x_der = Ga_times_p > ALMOST_ZERO ? _G.row(index)(coord_i) : 0.0;
          double f_x = (eyea(index) - p(index)) * (eyea(index) - p(index));
          double f_x_der = index == coord_i ? -2. * (eyea(index) - p(index)) : 0.0;

          double quotient_rule = (g_x * f_x_der - f_x * g_x_der) / (g_x * g_x);

          sum += quotient_rule;
        }

        g[coord_i] = _gamma < GAMMA_CUTOFF ? sum : sum / _gamma;
      }

      g[_fhat.size()] = _gamma < GAMMA_CUTOFF ? -1.0 * _gamma : -1.0;
    }
    else if (i == _fhat.size())
    {
      // sum
      g.ones();
      g = -1.0 * g;

      g[_fhat.size()] = 0.0;
    }
    else if (i == _fhat.size() + 1)
    {
      g.ones();
      g[_fhat.size()] = 0.0;
    }
    else if (i > _fhat.size() + 1 && i < 2 * _fhat.size() + 2)
    {
      g.zeros();
      size_t index = i - _fhat.size() - 2;
      for (size_t j = 0; j < _fhat.size(); ++j) { g(j) = -1.0 * _G(index, j); }
      g[_fhat.size()] = 0.0;
    }
    else if (i >= 2 * _fhat.size() + 2)
    {
      size_t index = i - 2 * _fhat.size() - 2;
      // all positives
      g.zeros();
      g(index) = -1.0;
    }
  }
};

std::vector<VW::cb_graph_feedback::triplet> get_valid_graph(
    const std::vector<VW::cb_graph_feedback::triplet>& triplets, size_t dim)
{
  std::vector<VW::cb_graph_feedback::triplet> valid_triplets;
  for (auto& triplet : triplets)
  {
    if (triplet.row < dim && triplet.col < dim) { valid_triplets.push_back(triplet); }
  }

  return valid_triplets;
  ;
}

std::pair<arma::mat, arma::vec> set_initial_coordinates(const arma::vec& fhat, double gamma)
{
  // find fhat min
  auto min_fhat = fhat.min();
  arma::vec positivefhat = (fhat - min_fhat);

  // initial p can be uniform random
  arma::mat coordinates(positivefhat.size() + 1, 1);
  for (size_t i = 0; i < positivefhat.size(); i++) { coordinates[i] = 1.0 / positivefhat.size(); }

  // initial z
  double z = gamma < GAMMA_CUTOFF ? 1.0 : (1.0 / gamma);

  coordinates[positivefhat.size()] = z;

  return {coordinates, positivefhat};
}

arma::vec get_probs_from_coordinates(arma::mat& coordinates, VW::workspace& all)
{
  // constraints are enforcers but they can be broken, so we need to check that probs are positive and sum to 1

  size_t num_actions = coordinates.n_rows - 1;

  for (size_t i = 0; i < num_actions; i++)
  {
    if (std::isnan(coordinates[i])) { continue; }
    // clip to [0,1]
    coordinates[i] = std::max(0., std::min(coordinates[i], 1.));
  }

  float p_sum = 0;
  for (size_t i = 0; i < num_actions; i++) { p_sum += coordinates[i]; }

  // normalize
  for (size_t i = 0; i < num_actions; i++) { coordinates[i] = coordinates[i] / p_sum; }

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

void cb_explore_adf_graph_feedback::update_example_prediction(multi_ex& examples, const arma::sp_mat& G)
{
  auto& a_s = examples[0]->pred.a_s;
  arma::vec fhat(a_s.size());

  for (auto& as : a_s) { fhat(as.action) = as.score; }
  const double gamma = static_cast<double>(_gamma_scale) * static_cast<double>(std::pow(_counter, _gamma_exponent));

  auto coord_positivefhat = set_initial_coordinates(fhat, gamma);
  arma::mat coordinates = std::get<0>(coord_positivefhat);
  arma::vec positivefhat = std::get<1>(coord_positivefhat);

  _all->_fhat.clear();
  for (size_t i = 0; i < fhat.size(); i++) { _all->_fhat.push_back(fhat(i)); }
  _all->_gamma = gamma;

  ConstrainedFunctionType f(positivefhat, G, gamma);
  ens::AugLagrangian optimizer;
  optimizer.Optimize(f, coordinates);

  arma::vec probs = get_probs_from_coordinates(coordinates, *_all);
  // probs.print("probs");
  // std::cout << "z: " << coordinates[positivefhat.size()] << std::endl;

  // set the new probabilities in the example
  for (auto& as : a_s) { as.score = probs(as.action); }
  std::sort(
      a_s.begin(), a_s.end(), [](const VW::action_score& a, const VW::action_score& b) { return a.score > b.score; });
}

arma::sp_mat get_graph(const VW::cb_graph_feedback::reduction_features& graph_reduction_features, size_t num_actions,
    VW::io::logger& logger)
{
  arma::sp_mat G(num_actions, num_actions);

  auto valid_graph = get_valid_graph(graph_reduction_features.triplets, num_actions);
  if (valid_graph.size() != graph_reduction_features.triplets.size())
  {
    logger.warn("The graph provided is invalid, any coordinates that are out of bounds will be ignored");
  }

  arma::umat locations(2, valid_graph.size());

  arma::vec values(valid_graph.size());

  for (size_t i = 0; i < valid_graph.size(); i++)
  {
    const auto& triplet = valid_graph[i];
    locations(0, i) = triplet.row;
    locations(1, i) = triplet.col;
    values(i) = triplet.val;
  }

  return arma::sp_mat(true, locations, values, num_actions, num_actions);
}

template <bool is_learn>
void cb_explore_adf_graph_feedback::predict_or_learn_impl(VW::LEARNER::learner& base, multi_ex& examples)
{
  auto& graph_reduction_features =
      examples[0]->ex_reduction_features.template get<VW::cb_graph_feedback::reduction_features>();
  arma::sp_mat G = get_graph(graph_reduction_features, examples.size(), _all->logger);

  if (is_learn)
  {
    _counter++;
    std::vector<std::vector<VW::cb_class>> cb_labels;
    cb_labels.reserve(examples.size());

    // stash all of the labels
    for (size_t i = 0; i < examples.size(); i++)
    {
      cb_labels.emplace_back(std::move(examples[i]->l.cb.costs));
      examples[i]->l.cb.costs.clear();
    }

    auto restore_guard = VW::scope_exit(
        [&examples, &cb_labels]
        {
          for (size_t i = 0; i < examples.size(); i++) { examples[i]->l.cb.costs = std::move(cb_labels[i]); }
        });

    // re-instantiate the labels one-by-one and call learn
    for (size_t i = 0; i < examples.size(); i++)
    {
      auto* ex = examples[i];

      ex->l.cb.costs = std::move(cb_labels[i]);

      auto local_restore_guard = VW::scope_exit(
          [&ex, &cb_labels, &i]
          {
            cb_labels[i] = std::move(ex->l.cb.costs);
            ex->l.cb.costs.clear();
          });

      // if there is another label then learn, otherwise skip
      if (ex->l.cb.costs.size() > 0)
      {
        float stashed_probability = ex->l.cb.costs[0].probability;

        // calculate the probability for this action, if it is the action that was not chosen
        auto chosen_action = ex->l.cb.costs[0].action;
        auto current_action = i;

        if (chosen_action != current_action)
        {
          // get the graph probability
          auto graph_prob = G.row(chosen_action)(current_action);

          // sanity checks
          if (graph_prob == 0. || cb_labels[chosen_action].size() == 0 ||
              cb_labels[chosen_action][0].probability <= 0.f)
          {
            // this should not happen, input is probably wrong
            continue;
          }

          auto chosen_prob = cb_labels[chosen_action][0].probability;
          ex->l.cb.costs[0].probability = chosen_prob * graph_prob;
        }

        base.learn(examples);

        ex->l.cb.costs[0].probability = stashed_probability;
      }
    }
  }
  else
  {
    base.predict(examples);
    update_example_prediction(examples, G);
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

  std::stringstream msg;
  if (!read) { msg << "cb adf with graph feedback storing example counter:  = " << _counter << "\n"; }
  VW::details::bin_text_read_write_fixed_validated(
      io, reinterpret_cast<char*>(&_counter), sizeof(_counter), read, msg, text);
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
  float gamma_exponent = 0.5;

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

  auto data = VW::make_unique<explore_type>(
      all.output_runtime.global_metrics.are_metrics_enabled(), gamma_scale, gamma_exponent, &all);
  data->set_allow_multiple_costs(true);

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