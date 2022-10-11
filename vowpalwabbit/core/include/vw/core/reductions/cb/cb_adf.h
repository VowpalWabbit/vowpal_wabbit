// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/cb.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/vw_fwd.h"
#include "vw/core/vw_versions.h"

#include <vector>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* cb_adf_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW

// TODO: Move these functions into VW lib and not reductions
namespace CB_ADF  // NOLINT
{
VW::example* test_adf_sequence(const VW::multi_ex& ec_seq);
CB::cb_class get_observed_cost_or_default_cb_adf(const VW::multi_ex& examples);
struct cb_adf
{
  GEN_CS::cb_to_cs_adf gen_cs;
  void learn(VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq);
  void predict(VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq);
  bool update_statistics(const VW::example& ec, const VW::multi_ex& ec_seq);

  cb_adf(VW::cb_type_t cb_type, bool rank_all, float clip_p, bool no_predict, VW::workspace* all)
      : _no_predict(no_predict), _rank_all(rank_all), _clip_p(clip_p), _all(all)
  {
    gen_cs.cb_type = cb_type;
  }

  void set_scorer(VW::LEARNER::single_learner* scorer) { gen_cs.scorer = scorer; }

  bool get_rank_all() const { return _rank_all; }

  const GEN_CS::cb_to_cs_adf& get_gen_cs() const { return gen_cs; }
  GEN_CS::cb_to_cs_adf& get_gen_cs() { return gen_cs; }

  const VW::version_struct* get_model_file_ver() const { return &_all->model_file_ver; }

  bool learn_returns_prediction() const
  {
    return ((gen_cs.cb_type == VW::cb_type_t::mtr) && !_no_predict) || gen_cs.cb_type == VW::cb_type_t::ips ||
        gen_cs.cb_type == VW::cb_type_t::dr || gen_cs.cb_type == VW::cb_type_t::dm ||
        gen_cs.cb_type == VW::cb_type_t::sm;
  }

  CB::cb_class* known_cost() { return &gen_cs.known_cost; }

private:
  void learn_ips(LEARNER::multi_learner& base, VW::multi_ex& examples);
  void learn_dr(LEARNER::multi_learner& base, VW::multi_ex& examples);
  void learn_dm(LEARNER::multi_learner& base, VW::multi_ex& examples);
  void learn_sm(LEARNER::multi_learner& base, VW::multi_ex& examples);
  template <bool predict>
  void learn_mtr(LEARNER::multi_learner& base, VW::multi_ex& examples);

private:
  std::vector<CB::label> _cb_labels;
  VW::cs_label _cs_labels;
  std::vector<VW::cs_label> _prepped_cs_labels;

  VW::action_scores _a_s;              // temporary storage for mtr and sm
  VW::action_scores _a_s_mtr_cs;       // temporary storage for mtr cost sensitive example
  VW::action_scores _prob_s;           // temporary storage for sm; stores softmax values
  VW::v_array<uint32_t> _backup_nf;    // temporary storage for sm; backup for numFeatures in examples
  VW::v_array<float> _backup_weights;  // temporary storage for sm; backup for weights in examples

  uint64_t _offset = 0;
  const bool _no_predict;
  const bool _rank_all;
  const float _clip_p;

  VW::workspace* _all = nullptr;
};
}  // namespace CB_ADF
