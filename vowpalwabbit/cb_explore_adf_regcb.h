/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

#include "cb_explore_adf_common.h"
#include "action_score.h"
#include "cb.h"
#include "reductions_fwd.h"

#include <vector>

namespace VW
{
namespace cb_explore_adf
{
namespace regcb
{
LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

struct cb_explore_adf_regcb : public cb_explore_adf_base
{
 private:
  size_t _counter;
  bool _regcbopt;  // use optimistic variant of RegCB
  float _c0;       // mellowness parameter for RegCB
  bool _first_only;
  float _min_cb_cost;
  float _max_cb_cost;

  std::vector<float> _min_costs;
  std::vector<float> _max_costs;

  // for backing up cb example data when computing sensitivities
  std::vector<ACTION_SCORE::action_scores> _ex_as;
  std::vector<v_array<CB::cb_class>> _ex_costs;

 public:
  cb_explore_adf_regcb(bool regcbopt, float c0, bool first_only, float min_cb_cost, float max_cb_cost);
  template <bool is_learn>
  static void predict_or_learn(cb_explore_adf_regcb& data, LEARNER::multi_learner& base, multi_ex& examples);
  ~cb_explore_adf_regcb() = default;

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);

  void get_cost_ranges(float delta, LEARNER::multi_learner& base, multi_ex& examples, bool min_only);
  float binary_search(float fhat, float delta, float sens, float tol = 1e-6);
};
}  // namespace regcb
}  // namespace cb_explore_adf
}  // namespace VW
