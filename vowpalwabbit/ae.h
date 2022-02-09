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
namespace ae
{
VW::LEARNER::base_learner* ae_setup(VW::setup_base_i&);

struct ae_score : scored_config
{
  ae_score() : VW::scored_config() {}
  ae_score(double alpha, double tau, uint64_t model_idx) : VW::scored_config(alpha, tau), _model_idx(model_idx) {}
  float get_upper_bound() const { return this->current_ips(); }
  float get_lower_bound() const { return _lower_bound; }
  uint64_t get_model_idx() const { return _model_idx; }
  void update_bounds(float w, float r);

  float _lower_bound = 0.f;
  uint64_t _model_idx;
};

struct ae_data
{
  ae_data(uint64_t num_configs, uint64_t min_scope, double ae_alpha, double ae_tau, parameters& weights)
      : min_scope(min_scope), ae_alpha(ae_alpha), ae_tau(ae_tau), weights(weights)
  {
    for (uint64_t i = 0; i < num_configs; ++i)
    {
      ae_score s(ae_alpha, ae_tau, i);
      scored_configs.push_back(s);
    }
  }
  std::vector<ae_score> scored_configs;
  uint64_t min_scope;
  double ae_alpha;
  double ae_tau;
  parameters& weights;
};

}  // namespace ae

namespace model_utils
{
size_t read_model_field(io_buf&, VW::ae::ae_score&);
size_t read_model_field(io_buf&, VW::ae::ae_data&);
size_t write_model_field(io_buf&, const VW::ae::ae_score&, const std::string&, bool);
size_t write_model_field(io_buf&, const VW::ae::ae_data&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
