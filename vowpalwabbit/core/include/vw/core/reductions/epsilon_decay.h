// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/array_parameters.h"
#include "vw/core/distributionally_robust.h"
#include "vw/core/reductions_fwd.h"
#include "vw/core/scored_config.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <numeric>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* epsilon_decay_setup(VW::setup_base_i&);

namespace epsilon_decay
{
struct epsilon_decay_score : scored_config
{
  epsilon_decay_score() = default;
  epsilon_decay_score(double alpha, double tau) : VW::scored_config(alpha, tau) {}
  float decayed_epsilon(uint64_t update_count);
  uint64_t get_score_idx() const { return _score_idx; }

  uint64_t _score_idx;
};

struct epsilon_decay_data
{
  epsilon_decay_data(uint64_t model_count, uint64_t min_scope, double epsilon_decay_alpha, double epsilon_decay_tau,
      dense_parameters& weights, VW::io::logger logger, bool log_champ_changes, bool constant_epsilon)
      : _min_scope(min_scope)
      , _epsilon_decay_alpha(epsilon_decay_alpha)
      , _epsilon_decay_tau(epsilon_decay_tau)
      , _weights(weights)
      , _logger(std::move(logger))
      , _log_champ_changes(log_champ_changes)
      , _constant_epsilon(constant_epsilon)
  {
    _scored_configs.reserve(model_count);
    _weight_indices.resize(model_count);
    std::iota(_weight_indices.begin(), _weight_indices.end(), 0);
    for (uint64_t i = 0; i < model_count; ++i)
    {
      _scored_configs.emplace_back(i + 1);
      for (uint64_t j = 0; j < i + 1; ++j)
      { _scored_configs.back().emplace_back(epsilon_decay_alpha, epsilon_decay_tau); }
    }
  }

  void update_weights(VW::LEARNER::multi_learner& base, VW::multi_ex& examples);
  void promote_model(int64_t model_ind, int64_t swap_dist);
  void rebalance_greater_models(int64_t model_ind, int64_t swap_dist, int64_t model_count);
  void clear_weights_and_scores(int64_t swap_dist, int64_t model_count);
  void shift_model(int64_t model_ind, int64_t swap_dist, int64_t model_count);
  void check_score_bounds();
  void check_horizon_bounds();

  std::vector<std::vector<epsilon_decay_score>> _scored_configs;
  std::vector<uint64_t> _weight_indices;
  uint64_t _min_scope;
  double _epsilon_decay_alpha;  // Confidence interval
  double _epsilon_decay_tau;    // Count decay time constant
  dense_parameters& _weights;
  VW::io::logger _logger;
  bool _log_champ_changes;
  bool _constant_epsilon;
};

}  // namespace epsilon_decay
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf&, VW::reductions::epsilon_decay::epsilon_decay_score&);
size_t read_model_field(io_buf&, VW::reductions::epsilon_decay::epsilon_decay_data&);
size_t write_model_field(io_buf&, const VW::reductions::epsilon_decay::epsilon_decay_score&, const std::string&, bool);
size_t write_model_field(io_buf&, const VW::reductions::epsilon_decay::epsilon_decay_data&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
