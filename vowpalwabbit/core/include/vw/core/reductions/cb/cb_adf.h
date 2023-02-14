// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/cb.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/vw_fwd.h"
#include "vw/core/vw_versions.h"

#include <memory>
#include <vector>

namespace VW
{
VW::example* test_cb_adf_sequence(const VW::multi_ex& ec_seq);
VW::cb_class get_observed_cost_or_default_cb_adf(const VW::multi_ex& examples);
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> cb_adf_setup(VW::setup_base_i& stack_builder);

class cb_adf
{
public:
  VW::details::cb_to_cs_adf gen_cs;

  void learn(VW::LEARNER::learner& base, VW::multi_ex& ec_seq);
  void predict(VW::LEARNER::learner& base, VW::multi_ex& ec_seq);
  bool update_statistics(const VW::example& ec, const VW::multi_ex& ec_seq, VW::shared_data& sd) const;

  cb_adf(VW::cb_type_t cb_type, bool rank_all, float clip_p, bool no_predict, VW::workspace* all)
      : _no_predict(no_predict), _rank_all(rank_all), _clip_p(clip_p), _all(all)
  {
    gen_cs.cb_type = cb_type;
  }

  void set_scorer(VW::LEARNER::learner* scorer) { gen_cs.scorer = scorer; }

  bool get_rank_all() const { return _rank_all; }

  const VW::details::cb_to_cs_adf& get_gen_cs() const { return gen_cs; }
  VW::details::cb_to_cs_adf& get_gen_cs() { return gen_cs; }

  const VW::version_struct* get_model_file_ver() const;

  bool learn_returns_prediction() const
  {
    return ((gen_cs.cb_type == VW::cb_type_t::MTR) && !_no_predict) || gen_cs.cb_type == VW::cb_type_t::IPS ||
        gen_cs.cb_type == VW::cb_type_t::DR || gen_cs.cb_type == VW::cb_type_t::DM ||
        gen_cs.cb_type == VW::cb_type_t::SM;
  }

  VW::cb_class* known_cost() { return &gen_cs.known_cost; }
  const VW::cb_class* known_cost() const { return &gen_cs.known_cost; }

private:
  void learn_ips(VW::LEARNER::learner& base, VW::multi_ex& examples);
  void learn_dr(VW::LEARNER::learner& base, VW::multi_ex& examples);
  void learn_dm(VW::LEARNER::learner& base, VW::multi_ex& examples);
  void learn_sm(VW::LEARNER::learner& base, VW::multi_ex& examples);
  template <bool predict>
  void learn_mtr(VW::LEARNER::learner& base, VW::multi_ex& examples);

private:
  std::vector<VW::cb_label> _cb_labels;
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
}  // namespace reductions
}  // namespace VW

namespace CB_ADF  // NOLINT
{
VW_DEPRECATED("Moved into VW namespace.") inline VW::example* test_adf_sequence(const VW::multi_ex& ec_seq)
{
  return VW::test_cb_adf_sequence(ec_seq);
}
VW_DEPRECATED("Moved into VW namespace.")
inline VW::cb_class get_observed_cost_or_default_cb_adf(const VW::multi_ex& examples)
{
  return VW::get_observed_cost_or_default_cb_adf(examples);
}
using cb_adf VW_DEPRECATED("Moved into VW namespace.") = VW::reductions::cb_adf;
}  // namespace CB_ADF
