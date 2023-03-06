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
  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::learner& base, multi_ex& examples);
  void learn(VW::LEARNER::learner& base, multi_ex& examples);
  void save_load(io_buf& io, bool read, bool text);
  size_t _counter = 0;

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::learner& base, multi_ex& examples);
  void update_example_prediction(multi_ex& examples);
};
}  // namespace cb_explore_adf

class ConstrainedFunctionType
{
  const arma::vec& _fhat;
  const arma::sp_mat& _Z;
  const float _gamma;

public:
  ConstrainedFunctionType(const arma::vec& scores, const arma::sp_mat& Z, const float gamma)
      : _fhat(scores), _Z(Z), _gamma(gamma)
  {
  }

  // Return the objective function f(x) for the given x.
  double Evaluate(const arma::mat& x)
  {
    arma::vec p(x.n_rows - 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p(i) = x(i); }

    float C = x(x.n_rows - 1);

    std::cout << "Evaluate:" << std::endl;

    p.print("p: ");
    _fhat.print("fhat: ");
    auto r = arma::dot(_fhat, p) + C;
    r = r / _gamma;
    std::cout << "C: " << C << std::endl;
    std::cout << "result: " << r << std::endl;
    return r;
  }

  // Compute the gradient of f(x) for the given x and store the result in g.
  void Gradient(const arma::mat&, arma::mat& g)
  {
    std::cout << "Gradient:" << std::endl;
    g = arma::vec(_fhat.n_rows + 1);
    for (size_t i = 0; i < _fhat.n_rows; ++i) { g(i) = _fhat(i); }
    g(_fhat.n_rows) = -1.f;
    g.print("g: ");
  }

  // Get the number of constraints on the objective function.
  size_t NumConstraints() { return _fhat.size() + 3; }

  // Evaluate constraint i at the parameters x.  If the constraint is
  // unsatisfied, a value greater than 0 should be returned.  If the constraint
  // is satisfied, 0 should be returned.  The optimizer will add this value to
  // its overall objective that it is trying to minimize.
  double EvaluateConstraint(const size_t i, const arma::mat& x)
  {
    std::cout << "Evaluate Constraint: " << i << std::endl;
    _Z.print("Z: ");

    arma::vec p(x.n_rows - 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p(i) = x(i); }
    p.print("p: ");

    float C = x(x.n_rows - 1);

    if (i < _fhat.size())
    {
      arma::vec eyea = arma::zeros<arma::vec>(p.n_rows);
      eyea(i) = 1.f;

      eyea.print("eyea: ");

      auto fhata = _fhat(i);
      std::cout << "fhat: " << fhata << " C: " << C << std::endl;

      double sum = 0.f;
      std::stringstream ss;
      ss << "sum: (";
      for (size_t index = 0; index < p.n_rows; index++)
      {
        ss << "index: " << index << " quad_over_lin(";
        arma::vec gammaGa(_Z.row(index).n_cols);
        for (size_t j = 0; j < _Z.row(index).n_cols; j++) { gammaGa(j) = _Z.row(index)(j); }
        gammaGa.print("gammaGa: " + std::to_string(index) + ": ");

        auto gammaGa_times_p = arma::dot(gammaGa, p);

        float denominator = gammaGa_times_p;  // _Z.row(index)(index) * p(index);
        auto nominator = (eyea(index) - p(index)) * (eyea(index) - p(index));
        sum += nominator / denominator;
        ss << eyea(index) << " - " << p(index) << ")^2 / " << denominator;
        ss << ") + ";
      }
      ss << ")" << std::endl;
      std::cout << ss.str() << std::endl;

      std::cout << "sum: " << sum << " fhata + C: " << fhata + C << std::endl;

      if (sum <= (fhata + C)) { return 0.f; }
      else
      {
        std::cout << "CONSTRAINT: " << i << " NOT MET, diff is: " << (sum - (fhata + C)) << std::endl;
        return (sum - (fhata + C));
      }
    }
    else if (i == _fhat.size())
    {
      std::cout << "sum: " << arma::sum(p) << std::endl;
      if (VW::math::are_same(static_cast<float>(arma::sum(p)), 1.f)) { return 0.f; }
      std::cout << "SUM CONSTRAINT NOT 1 but: " << std::abs(arma::sum(p) - 1.f) << std::endl;
      return 1.f * std::abs(arma::sum(p) - 1.f);
    }
    else if (i == _fhat.size() + 1)
    {
      if (arma::all(p >= 0.f)) { return 0.f; }

      std::cout << "SUMS NOT NONEG but: " << -1.f * arma::min(p) << std::endl;

      return -1.f * arma::min(p);
    }
    else if (i == _fhat.size() + 2)
    {
      if (C > 1.f) { return 0.f; }
      return std::abs(C);
    }
    return 0.f;
  }

  // Evaluate the gradient of constraint i at the parameters x, storing the
  // result in the given matrix g.  If the constraint is not satisfied, the
  // gradient should be set in such a way that the gradient points in the
  // direction where the constraint would be satisfied.
  void GradientConstraint(const size_t i, const arma::mat& x, arma::mat& g)
  {
    std::cout << "Gradient constraint: " << i << std::endl;

    arma::vec p(x.n_rows - 1);
    for (size_t i = 0; i < p.n_rows; ++i) { p(i) = x(i); }

    float C = x(x.n_rows - 1);

    if (i < _fhat.size())
    {
      g = arma::vec(_fhat.size() + 1);
      g.zeros();
      arma::vec eyea = arma::zeros<arma::vec>(p.n_rows);
      eyea(i) = 1.f;

      _fhat.print("fhat: ");
      eyea.print("eyea: ");

      for (size_t coord_i = 0; coord_i < _fhat.size(); coord_i++)
      {
        double sum = 0.f;
        std::stringstream ss;
        ss << "sum: (";
        for (size_t index = 0; index < p.n_rows; index++)
        {
          arma::vec gammaGa(_Z.row(index).n_cols);
          for (size_t j = 0; j < _Z.row(index).n_cols; j++) { gammaGa(j) = _Z.row(index)(j); }
          auto gammaGa_times_p = arma::dot(gammaGa, p);

          if (index == coord_i)
          {
            ss << "index: " << index << " quad_over_lin(";
            gammaGa.print("gammaGa: " + std::to_string(index) + ": ");

            float denominator = gammaGa_times_p;

            auto b = _Z.row(index)(index);
            auto c = gammaGa_times_p - _Z.row(index)(index) * p(index);
            auto nominator = -1.f * ((eyea(index) - p(index)) * (eyea(index) * b + b * p(index) + 2.f * c));

            sum += (nominator / (denominator * denominator));
            ss << eyea(index) << " - " << p(index) << ")^2 / " << denominator;
            ss << ") + ";
          }
          else
          {
            auto a = (eyea(index) - p(index)) * (eyea(index) - p(index));
            auto b = _Z.row(index)(coord_i);
            // auto c = gammaGa_times_p - b * p(coord_i);
            auto nominator = -1.f * ((a * b));
            auto denominator = gammaGa_times_p;
            sum += nominator / (denominator * denominator);
          }
        }

        std::cout << coord_i << " : " << sum << std::endl;

        g(coord_i) = sum;
      }
    }
    else if (i == _fhat.size())
    {
      g = arma::vec(_fhat.size() + 1);
      g.ones();
      g(_fhat.size()) = 0.f;
    }
    else if (i == _fhat.size() + 1)
    {
      g.ones();
      g = arma::vec(_fhat.size() + 1);
      g(_fhat.size()) = 0.f;
    }
    else if (i == _fhat.size() + 2)
    {
      g.zeros();
      g(_fhat.size()) = C > 0.f ? 1.f : (C < 0.f ? -1.f : 0.f);
    }
  }
};

void cb_explore_adf_graph_feedback::update_example_prediction(multi_ex& examples)
{
  // TODO store Z in reduction data and only update when new Z is provided

  // get fhat
  // using action_scores = VW::v_array<action_score>;
  auto& a_s = examples[0]->pred.a_s;
  arma::vec fhat(a_s.size());

  for (auto& as : a_s) { fhat(as.action) = as.score; }
  fhat.print("fhat before: ");

  float fhat_min = *std::min_element(fhat.begin(), fhat.end());

  for (auto& as : a_s) { fhat(as.action) = as.score - fhat_min; }

  auto _gamma_scale = 10.f;
  auto _gamma_exponent = 0.5f;
  const float gamma = _gamma_scale * static_cast<float>(std::pow(_counter, _gamma_exponent));

  // for (size_t i = 0; i < fhat.n_rows; i++) { fhat(i) = fhat(i) * gamma; }
  fhat.print("fhat after: ");

  float C = 1.f + fhat_min;
  // C = gamma * C;
  std::cout << "fhat_min: " << fhat_min << " C: " << C << std::endl;

  // initial p can be uniform random
  arma::vec coordinates(fhat.size() + 1);
  for (size_t i = 0; i < fhat.size(); i++) { coordinates(i) = 1.f / fhat.size(); }
  coordinates(fhat.size()) = C;

  // get Z
  auto& graph_reduction_features =
      examples[0]->ex_reduction_features.template get<VW::cb_graph_feedback::reduction_features>();

  if (graph_reduction_features.triplets.size() == 0)
  {
    std::cout << "do something when there is no Z defined!" << std::endl;
  }

  arma::umat locations(2, graph_reduction_features.triplets.size());

  arma::vec values(graph_reduction_features.triplets.size());

  for (size_t i = 0; i < graph_reduction_features.triplets.size(); i++)
  {
    const auto& triplet = graph_reduction_features.triplets[i];
    locations(0, i) = triplet.row;
    locations(1, i) = triplet.col;
    values(i) = triplet.val;
  }

  arma::sp_mat Z(true, locations, values, a_s.size(), a_s.size());

  Z.print("Z: ");

  coordinates.print("coords ur: ");

  ConstrainedFunctionType f(fhat, Z, gamma);

  std::cout << "new optimizer" << std::endl;
  ens::AugLagrangian optimizer;
  optimizer.MaxIterations() = 0;
  std::cout << "MAX ITERS: " << optimizer.MaxIterations() << std::endl;
  optimizer.Optimize(f, coordinates);
  std::cout << "gamma: " << gamma << std::endl;
  coordinates.print("coords: ");
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

  config::option_group_definition new_options(
      "[Reduction] Experimental: Contextual Bandit Exploration with ADF with graph feedback");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
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

  auto data = VW::make_unique<explore_type>(with_metrics);

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