// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_large_action_space.h"

#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "cb_adf.h"
#include "cb_explore.h"
#include "config/options.h"
#include "explore.h"
#include "gd_predict.h"
#include "gen_cs_example.h"
#include "label_parser.h"
#include "rand48.h"
#include "setup_base.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
using namespace VW::cb_explore_adf;
using SparseM = Eigen::SparseMatrix<float>;
using Triplet = Eigen::Triplet<float>;

namespace
{
struct cb_explore_adf_large_action_space
{
private:
  uint64_t d = 0;
  float gamma = 1;
  // TODO initialize seed
  uint64_t seed = 0;
  VW::workspace* all;

public:
  cb_explore_adf_large_action_space(uint64_t d_, float gamma_, VW::workspace* all_);
  ~cb_explore_adf_large_action_space() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples);
};

cb_explore_adf_large_action_space::cb_explore_adf_large_action_space(uint64_t d_, float gamma_, VW::workspace* all_)
    : d(d_), gamma(gamma_), all(all_)
{
}

struct LazyGaussianVector
{
private:
  uint64_t column_index;
  uint64_t seed;

public:
  LazyGaussianVector(uint64_t column_index_, uint64_t seed_) : column_index(column_index_), seed(seed_) {}
  inline float operator[](uint64_t index) const
  {
    auto combined_index = index + column_index + seed;
    return merand48_boxmuller(combined_index);
  }
};

inline void vec_add(float& p, float fx, float fw) { p += fx * fw; }

template <bool is_learn>
void cb_explore_adf_large_action_space::predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  // Explore uniform random an epsilon fraction of the time.
  if (is_learn) { base.learn(examples); }
  else
  {
    base.predict(examples);

    auto& preds = examples[0]->pred.a_s;
    float min_ck = std::min_element(preds.begin(), preds.end(),
        [](ACTION_SCORE::action_score& a, ACTION_SCORE::action_score& b) { return a.score < b.score; })
                       ->score;

    std::vector<float> denominators;

    for (const auto& pred : preds)
    { denominators.push_back(std::sqrt(1 + d + (gamma / 4.0f * d) * (pred.score - min_ck))); }

    // create Q matrix with dimenstions Kxd where K = examples.size()
    uint64_t num_actions = examples[0]->pred.a_s.size();
    Eigen::MatrixXf Q(num_actions, d);

    // TODO extend wildspace interactions before calling foreach
    uint64_t row_index = 0;
    for (auto* ex : examples)
    {
      if (!CB::ec_is_example_header(*ex))
      {
        for (size_t col = 0; col < d; col++)
        {
          LazyGaussianVector w(col, seed);

          float dot_product = 0.f;
          GD::foreach_feature<float, float, vec_add, LazyGaussian>(w, all->ignore_some_linear, all->ignore_linear,
              all->interactions, all->extent_interactions, all->permutations, *ex, dot_product,
              all->_generate_interactions_object_cache);

          Q(row_index, col) = dot_product;
        }
        row_index++;
      }
    }

    std::cout << "here is a Q: " << std::endl;
    std::cout << Q << std::endl;
    std::cout << "-----" << std::endl;
  }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::cb_explore_adf_large_action_space_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool large_action_space = false;
  uint64_t d;
  float gamma;

  config::option_group_definition new_options(
      "[Reduction] Contextual Bandit Exploration with ADF with large action space");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("large_action_space", large_action_space)
               .necessary()
               .keep()
               .help("Large action space exploration"))
      .add(make_option("d", d).keep().allow_override().default_value(50).help("Max number of actions to explore"))
      .add(make_option("gamma", gamma).keep().allow_override().help("Gamma hyperparameter"));

  auto enabled = options.add_parse_and_check_necessary(new_options) && large_action_space;
  if (!enabled) { return nullptr; }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    // TODO make sure this is MTR
    options.insert("cb_adf", "");
    options.insert("no_predict", "");
  }

  size_t problem_multiplier = 1;

  VW::LEARNER::multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  bool with_metrics = options.was_supplied("extra_metrics");

  using explore_type = cb_explore_adf_base<cb_explore_adf_large_action_space>;
  auto data = VW::make_unique<explore_type>(with_metrics, d, gamma, &all);

  auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_large_action_space_setup))
                .set_input_label_type(VW::label_type_t::cb)
                .set_output_label_type(VW::label_type_t::cb)
                .set_input_prediction_type(VW::prediction_type_t::action_scores)
                .set_output_prediction_type(VW::prediction_type_t::action_probs)
                .set_params_per_weight(problem_multiplier)
                .set_finish_example(explore_type::finish_multiline_example)
                .set_print_example(explore_type::print_multiline_example)
                .set_persist_metrics(explore_type::persist_metrics)
                .build(&all.logger);
  return make_base(*l);
}
