// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#define EIGEN_MPL2_ONLY

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

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <armadillo>
#include <cmath>
#include <ensmallen.hpp>
#include <functional>
#include <vector>

using namespace Eigen;

using namespace VW::cb_explore_adf;

namespace VW
{
namespace cb_explore_adf
{
class cb_explore_adf_graph_feedback
{
public:
  cb_explore_adf_graph_feedback(float gamma) : _gamma(gamma) {}
  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::learner& base, multi_ex& examples);
  void learn(VW::LEARNER::learner& base, multi_ex& examples);
  void save_load(io_buf& io, bool read, bool text);
  size_t _counter = 0;
  float _gamma;

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::learner& base, multi_ex& examples);
  void update_example_prediction(multi_ex& examples);
};
}  // namespace cb_explore_adf

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
  double Evaluate(const arma::mat& x)
  {
    arma::mat p(x.n_rows - 1, 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p[i] = x[i]; }

    float z = x[x.n_rows - 1];

    return arma::dot(p, _fhat) + z;
  }

  // Compute the gradient of f(x) for the given x and store the result in g.
  void Gradient(const arma::mat&, arma::mat& g)
  {
    g.set_size(_fhat.n_rows + 1, 1);
    for (size_t i = 0; i < _fhat.n_rows; ++i) { g[i] = _fhat(i); }
    g[_fhat.n_rows] = 1.f;
  }

  // Get the number of constraints on the objective function.
  size_t NumConstraints() { return _fhat.size() + 4; }

  // Evaluate constraint i at the parameters x.  If the constraint is
  // unsatisfied, a value greater than 0 should be returned.  If the constraint
  // is satisfied, 0 should be returned.  The optimizer will add this value to
  // its overall objective that it is trying to minimize.

  double EvaluateConstraint(const size_t i, const arma::mat& x)
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
      if (z > 1.f) { return 0.f; }
      return 1.f - z;
    }
    return 0.;
  }

  // Evaluate the gradient of constraint i at the parameters x, storing the
  // result in the given matrix g.  If the constraint is not satisfied, the
  // gradient should be set in such a way that the gradient points in the
  // direction where the constraint would be satisfied.
  void GradientConstraint(const size_t i, const arma::mat& x, arma::mat& g)
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

      if (z < 1.f) { g[_fhat.size()] = -1.f; }
    }
  }
};

void cb_explore_adf_graph_feedback::update_example_prediction(multi_ex& examples)
{
  // TODO store G in reduction data and only update when new G is provided ?

  // get fhat
  auto& a_s = examples[0]->pred.a_s;
  arma::vec fhat(a_s.size());
  for (auto& as : a_s) { fhat(as.action) = as.score; }

  // initial p can be uniform random
  arma::mat coordinates(fhat.size() + 1, 1);
  for (size_t i = 0; i < fhat.size(); i++) { coordinates[i] = 1.f / fhat.size(); }

  // initial z can be 1
  float z = 1.f;

  coordinates[fhat.size()] = z;

  // get G
  auto& graph_reduction_features =
      examples[0]->ex_reduction_features.template get<VW::cb_graph_feedback::reduction_features>();

  if (graph_reduction_features.triplets.size() == 0)
  {
    std::cout << "do something when there is no G defined!" << std::endl;
  }

  arma::umat locations(2, graph_reduction_features.triplets.size());

  arma::vec values(graph_reduction_features.triplets.size());

  for (size_t i = 0; i < graph_reduction_features.triplets.size(); i++)
  {
    const auto& triplet = graph_reduction_features.triplets[i];
    locations(0, i) = triplet.row;
    locations(1, i) = triplet.col;
    values(i) = triplet.val * _gamma;
  }

  arma::sp_mat G(true, locations, values, a_s.size(), a_s.size());

  ConstrainedFunctionType f(fhat, G, _gamma);

  ens::AugLagrangian optimizer;
  // optimizer.MaxIterations() = 1000;
  optimizer.Optimize(f, coordinates);

  float p_sum = 0;
  for (size_t i = 0; i < a_s.size(); i++) { p_sum += coordinates[i]; }

  coordinates.print("coords before balancing: ");

  if (!VW::math::are_same(p_sum, 1.f))
  {
    float rest = 1.f - p_sum;
    float rest_each = rest / (a_s.size());
    for (size_t i = 0; i < a_s.size(); i++) { coordinates[i] = coordinates[i] + rest_each; }
  }

  coordinates.print("coords: ");

  {
    arma::vec probs(coordinates.n_rows - 1);
    for (size_t i = 0; i < probs.n_rows; ++i)
    {
      probs(i) = coordinates[i];
      if (std::isnan(probs(i))) { return; }
    }
    probs.print("final p: ");

    float z = coordinates[coordinates.n_rows - 1];
    std::cout << "VALUE IS: " << arma::dot(fhat, probs) + z << std::endl;
  }
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

void cb_explore_adf_graph_feedback::save_load(io_buf&, bool, bool) {}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_explore_adf_graph_feedback_setup(
    VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool graph_feedback = false;
  float gamma = 0.f;

  config::option_group_definition new_options(
      "[Reduction] Experimental: Contextual Bandit Exploration with ADF with graph feedback");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("gamma", gamma).keep().default_value(10.f))
      .add(make_option("graph_feedback", graph_feedback).necessary().keep().help("Graph feedback pdf").experimental());

  auto enabled = options.add_parse_and_check_necessary(new_options) && graph_feedback;
  if (!enabled) { return nullptr; }

  if (options.was_supplied("cb_type"))
  {
    auto cb_type = options.get_typed_option<std::string>("cb_type").value();
    if (cb_type != "mtr")
    {
      all.logger.err_warn(
          "Only cb_type 'mtr' is currently supported with graph feedback, resetting to mtr. Input was: '{}'", cb_type);
      options.replace("cb_type", "mtr");
    }
  }

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::cb_label_parser_global;

  using explore_type = cb_explore_adf_base<cb_explore_adf_graph_feedback>;

  size_t problem_multiplier = 1;
  bool with_metrics = options.was_supplied("extra_metrics");

  auto data = VW::make_unique<explore_type>(with_metrics, gamma);

  auto l = VW::LEARNER::make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(VW::reductions::cb_explore_adf_graph_feedback_setup))
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_params_per_weight(problem_multiplier)
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