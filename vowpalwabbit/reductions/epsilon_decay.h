// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "reductions_fwd.h"
#include "distributionally_robust.h"
#include "scored_config.h"
#include "array_parameters.h"

namespace VW
{
namespace epsilon_decay
{
VW::LEARNER::base_learner* epsilon_decay_setup(VW::setup_base_i&);

struct epsilon_decay_score : scored_config
{
  epsilon_decay_score() : VW::scored_config() {}
  epsilon_decay_score(double alpha, double tau, uint64_t model_idx)
      : VW::scored_config(alpha, tau), _model_idx(model_idx)
  {
  }
  float get_upper_bound() const { return this->current_ips(); }
  float get_lower_bound() const { return _lower_bound; }
  uint64_t get_model_idx() const { return _model_idx; }
  void update_bounds(float w, float r);

  float _lower_bound = 0.f;
  uint64_t _model_idx;
};

struct epsilon_decay_data
{
  epsilon_decay_data(uint64_t num_configs, uint64_t min_scope, double epsilon_decay_alpha, double epsilon_decay_tau,
      parameters& weights)
      : min_scope(min_scope)
      , epsilon_decay_alpha(epsilon_decay_alpha)
      , epsilon_decay_tau(epsilon_decay_tau)
      , weights(weights)
  {
    for (uint64_t i = 0; i < num_configs; ++i)
    {
      epsilon_decay_score s(epsilon_decay_alpha, epsilon_decay_tau, i);
      scored_configs.push_back(s);
    }
  }
  std::vector<epsilon_decay_score> scored_configs;
  uint64_t min_scope;
  double epsilon_decay_alpha;
  double epsilon_decay_tau;
  parameters& weights;
};

}  // namespace epsilon_decay

namespace model_utils
{
size_t read_model_field(io_buf&, VW::epsilon_decay::epsilon_decay_score&);
size_t read_model_field(io_buf&, VW::epsilon_decay::epsilon_decay_data&);
size_t write_model_field(io_buf&, const VW::epsilon_decay::epsilon_decay_score&, const std::string&, bool);
size_t write_model_field(io_buf&, const VW::epsilon_decay::epsilon_decay_data&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
