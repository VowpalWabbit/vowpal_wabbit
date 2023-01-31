// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/cb.h"
#include "vw/core/learner_fwd.h"
#include "vw/core/version.h"
#include "vw/core/vw_fwd.h"

#include <vector>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* cb_explore_adf_squarecb_setup(VW::setup_base_i& stack_builder);

class cb_explore_adf_squarecb
{
public:
  cb_explore_adf_squarecb(float gamma_scale, float gamma_exponent, bool elim, float c0, float min_cb_cost,
      float max_cb_cost, VW::version_struct model_file_version, float epsilon);
  ~cb_explore_adf_squarecb() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, VW::multi_ex& examples);
  void learn(VW::LEARNER::multi_learner& base, VW::multi_ex& examples);
  void save_load(VW::io_buf& io, bool read, bool text);

  float get_gamma();

private:
  size_t _counter;
  float _gamma_scale;     // Scale factor for SquareCB reediness parameter $\gamma$.
  float _gamma_exponent;  // Exponent on $t$ for SquareCB reediness parameter $\gamma$.

  // Parameters and data structures for RegCB action set computation
  bool _elim;
  float _c0;
  float _min_cb_cost;
  float _max_cb_cost;
  float _epsilon;

  std::vector<float> _min_costs;
  std::vector<float> _max_costs;

  VW::version_struct _model_file_version;

  // for backing up cb example data when computing sensitivities
  std::vector<VW::action_scores> _ex_as;
  std::vector<std::vector<VW::cb_class>> _ex_costs;
  void get_cost_ranges(float delta, VW::LEARNER::multi_learner& base, VW::multi_ex& examples, bool min_only);
  float binary_search(float fhat, float delta, float sens, float tol = 1e-6);
};
}  // namespace reductions
}  // namespace VW