// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/estimators/confidence_sequence_robust.h"
#include "vw/core/io_buf.h"
#include "vw/core/learner_fwd.h"
#include "vw/core/vw_fwd.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> epsilon_decay_setup(VW::setup_base_i&);

namespace epsilon_decay
{
float decayed_epsilon(float init_ep, uint64_t update_count);

class epsilon_decay_data
{
public:
  epsilon_decay_data(uint64_t model_count, uint64_t min_scope, double epsilon_decay_significance_level,
      double epsilon_decay_estimator_decay, dense_parameters& weights, std::string epsilon_decay_audit_str,
      bool constant_epsilon, uint32_t& wpp, uint64_t _min_champ_examples, float initial_epsilon,
      uint64_t shift_model_bounds, bool reward_as_cost, double tol_x, bool is_brentq, bool predict_only_model);
  void update_weights(float init_ep, VW::LEARNER::learner& base, VW::multi_ex& examples);
  void promote_model(int64_t model_ind, int64_t swap_dist);
  void rebalance_greater_models(int64_t model_ind, int64_t swap_dist, int64_t model_count);
  void clear_weights_and_estimators(int64_t swap_dist, int64_t model_count);
  void shift_model(int64_t model_ind, int64_t swap_dist, int64_t model_count);
  void check_estimator_bounds();
  void check_horizon_bounds();

  std::vector<std::vector<VW::estimators::confidence_sequence_robust>> conf_seq_estimators;
  std::vector<uint64_t> _weight_indices;
  uint64_t _model_count;
  uint64_t _min_scope;
  double _epsilon_decay_significance_level;  // Confidence interval
  double _epsilon_decay_estimator_decay;     // Count decay time constant
  dense_parameters& _weights;
  std::string _epsilon_decay_audit_str;
  std::stringstream _audit_msg;
  uint64_t _global_counter = 1;
  bool _constant_epsilon;
  uint32_t& _wpp;
  uint64_t _min_champ_examples;
  float _initial_epsilon;
  uint64_t _shift_model_bounds;
  bool _reward_as_cost;
  bool _predict_only_model;

  // TODO: delete all this, gd and cb_adf must respect ft_offset, see header import of automl.cc
  std::vector<double> per_live_model_state_double;
  std::vector<uint64_t> per_live_model_state_uint64;
  double* _gd_normalized = nullptr;
  double* _gd_total_weight = nullptr;
  double* _sd_gravity = nullptr;
  uint64_t* _cb_adf_event_sum = nullptr;
  uint64_t* _cb_adf_action_sum = nullptr;
};

}  // namespace epsilon_decay
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf&, VW::reductions::epsilon_decay::epsilon_decay_data&);
size_t write_model_field(io_buf&, const VW::reductions::epsilon_decay::epsilon_decay_data&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
