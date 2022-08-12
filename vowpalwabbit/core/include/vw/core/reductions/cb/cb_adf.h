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
namespace CB_ADF
{
VW::example* test_adf_sequence(const VW::multi_ex& ec_seq);
CB::cb_class get_observed_cost_or_default_cb_adf(const VW::multi_ex& examples);
struct cb_adf
{
private:
  shared_data* _sd;
  // model_file_ver is only used to conditionally run save_load(). In the setup function
  // model_file_ver is not always set.
  VW::version_struct* _model_file_ver;

  std::vector<CB::label> _cb_labels;
  COST_SENSITIVE::label _cs_labels;
  std::vector<COST_SENSITIVE::label> _prepped_cs_labels;

  ACTION_SCORE::action_scores _a_s;         // temporary storage for mtr and sm
  ACTION_SCORE::action_scores _a_s_mtr_cs;  // temporary storage for mtr cost sensitive example
  ACTION_SCORE::action_scores _prob_s;      // temporary storage for sm; stores softmax values
  VW::v_array<uint32_t> _backup_nf;         // temporary storage for sm; backup for numFeatures in examples
  VW::v_array<float> _backup_weights;       // temporary storage for sm; backup for weights in examples

  uint64_t _offset;
  const bool _no_predict;
  const bool _rank_all;
  const float _clip_p;

  VW::io::logger logger;

public:
  GEN_CS::cb_to_cs_adf _gen_cs;
  void learn(VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq);
  void predict(VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq);
  bool update_statistics(const VW::example& ec, const VW::multi_ex& ec_seq);

  cb_adf(shared_data* sd, VW::cb_type_t cb_type, VW::version_struct* model_file_ver, bool rank_all, float clip_p,
      bool no_predict, VW::io::logger logger)
      : _sd(sd)
      , _model_file_ver(model_file_ver)
      , _offset(0)
      , _no_predict(no_predict)
      , _rank_all(rank_all)
      , _clip_p(clip_p)
      , logger(std::move(logger))
  {
    _gen_cs.cb_type = cb_type;
  }

  void set_scorer(VW::LEARNER::single_learner* scorer) { _gen_cs.scorer = scorer; }

  bool get_rank_all() const { return _rank_all; }

  const GEN_CS::cb_to_cs_adf& get_gen_cs() const { return _gen_cs; }
  GEN_CS::cb_to_cs_adf& get_gen_cs() { return _gen_cs; }

  const VW::version_struct* get_model_file_ver() const { return _model_file_ver; }

  bool learn_returns_prediction() const
  {
    return ((_gen_cs.cb_type == VW::cb_type_t::mtr) && !_no_predict) || _gen_cs.cb_type == VW::cb_type_t::ips ||
        _gen_cs.cb_type == VW::cb_type_t::dr || _gen_cs.cb_type == VW::cb_type_t::dm ||
        _gen_cs.cb_type == VW::cb_type_t::sm;
  }

  CB::cb_class* known_cost() { return &_gen_cs.known_cost; }

private:
  void learn_IPS(LEARNER::multi_learner& base, VW::multi_ex& examples);
  void learn_DR(LEARNER::multi_learner& base, VW::multi_ex& examples);
  void learn_DM(LEARNER::multi_learner& base, VW::multi_ex& examples);
  void learn_SM(LEARNER::multi_learner& base, VW::multi_ex& examples);
  template <bool predict>
  void learn_MTR(LEARNER::multi_learner& base, VW::multi_ex& examples);
};
}  // namespace CB_ADF
