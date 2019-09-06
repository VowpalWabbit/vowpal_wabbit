/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

#include <stdint.h>
#include <vector>

#include "cb_explore_adf_common.h"
#include "cb.h"
#include "cost_sensitive.h"
#include "v_array.h"
#include "action_score.h"
#include "gen_cs_example.h"
#include "reductions_fwd.h"

namespace VW
{
namespace cb_explore_adf
{
namespace cover
{
LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

struct cb_explore_adf_cover : public cb_explore_adf_base
{
 private:
  size_t _cover_size;
  float _psi;
  bool _nounif;
  bool _first_only;
  size_t _counter;
  LEARNER::multi_learner* _cs_ldf_learner;
  GEN_CS::cb_to_cs_adf _gen_cs;

  v_array<ACTION_SCORE::action_score> _action_probs;
  std::vector<float> _scores;
  COST_SENSITIVE::label _cs_labels;
  COST_SENSITIVE::label _cs_labels_2;
  v_array<COST_SENSITIVE::label> _prepped_cs_labels;
  v_array<CB::label> _cb_labels;

 public:
  cb_explore_adf_cover(
      size_t cover_size, float psi, bool nounif, bool first_only, LEARNER::multi_learner* cs_ldf_learner, LEARNER::single_learner * scorer, size_t cb_type
  );
  template <bool is_learn>
  static void predict_or_learn(cb_explore_adf_cover& data, LEARNER::multi_learner& base, multi_ex& examples);
  ~cb_explore_adf_cover();

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};

}  // namespace cover
}  // namespace cb_explore_adf
}  // namespace VW
