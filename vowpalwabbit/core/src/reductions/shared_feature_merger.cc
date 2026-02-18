// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/shared_feature_merger.h"

#include "vw/config/options.h"
#include "vw/core/cb.h"
#include "vw/core/example.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/learner.h"
#include "vw/core/scope_exit.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"

#include <iterator>
#include <string>
#include <vector>
namespace
{
class sfm_metrics
{
public:
  size_t count_learn_example_with_shared = 0;
};

class sfm_data
{
public:
  std::unique_ptr<sfm_metrics> metrics;
  VW::label_type_t label_type = VW::label_type_t::CB;
  bool store_shared_ex_in_reduction_features = false;
};

template <bool is_learn, bool is_cb_with_observations>
void predict_or_learn(sfm_data& data, VW::LEARNER::learner& base, VW::multi_ex& ec_seq)
{
  if (ec_seq.empty()) THROW("cb_adf: At least one action must be provided for an example to be valid.");

  VW::multi_ex::value_type shared_example = nullptr;
  const bool store_shared_ex_in_reduction_features = data.store_shared_ex_in_reduction_features;

  const bool has_example_header = VW::LEARNER::ec_is_example_header(*ec_seq[0], data.label_type);

  if (has_example_header)
  {
    shared_example = ec_seq[0];
    ec_seq.erase(ec_seq.begin());

    // merge sequences
    for (auto& example : ec_seq)
    {
      if (is_cb_with_observations)
      {
        if (example->l.cb_with_observations.is_observation) { continue; }
      }

      VW::details::append_example_namespaces_from_example(*example, *shared_example);
    }

    std::swap(ec_seq[0]->pred, shared_example->pred);
    std::swap(ec_seq[0]->tag, shared_example->tag);
    std::swap(ec_seq[0]->ex_reduction_features, shared_example->ex_reduction_features);
    if (store_shared_ex_in_reduction_features)
    {
      auto& red_features =
          ec_seq[0]->ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
      red_features.shared_example = shared_example;
    }
  }

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit(
      [has_example_header, &shared_example, &ec_seq, &store_shared_ex_in_reduction_features]
      {
        if (has_example_header)
        {
          for (auto& example : ec_seq)
          {
            if (is_cb_with_observations)
            {
              if (example->l.cb_with_observations.is_observation) { continue; }
            }

            VW::details::truncate_example_namespaces_from_example(*example, *shared_example);
          }
          std::swap(shared_example->pred, ec_seq[0]->pred);
          std::swap(shared_example->tag, ec_seq[0]->tag);
          std::swap(shared_example->ex_reduction_features, ec_seq[0]->ex_reduction_features);
          if (store_shared_ex_in_reduction_features)
          {
            auto& red_features =
                ec_seq[0]->ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
            red_features.reset_to_default();
          }
          ec_seq.insert(ec_seq.begin(), shared_example);
        }
      });

  if (ec_seq.empty()) { return; }
  if (is_learn) { base.learn(ec_seq); }
  else { base.predict(ec_seq); }

  if (data.metrics)
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_COND_CONST_EXPR
    if (is_learn && has_example_header) { data.metrics->count_learn_example_with_shared++; }
    VW_WARNING_STATE_POP
  }
}

void persist(sfm_data& data, VW::metric_sink& metrics)
{
  if (data.metrics)
  {
    metrics.set_uint("sfm_count_learn_example_with_shared", data.metrics->count_learn_example_with_shared);
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::shared_feature_merger_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto base = stack_builder.setup_base_learner();
  if (base == nullptr) { return nullptr; }
  std::set<label_type_t> sfm_labels = {label_type_t::CB, label_type_t::CS, label_type_t::CB_WITH_OBSERVATIONS};
  if (sfm_labels.find(base->get_input_label_type()) == sfm_labels.end() || !base->is_multiline()) { return base; }

  auto data = VW::make_unique<sfm_data>();
  if (all.output_runtime.global_metrics.are_metrics_enabled()) { data->metrics = VW::make_unique<sfm_metrics>(); }
  if (options.was_supplied("large_action_space")) { data->store_shared_ex_in_reduction_features = true; }

  auto multi_base = VW::LEARNER::require_multiline(base);
  data->label_type = base->get_input_label_type();

  std::shared_ptr<VW::LEARNER::learner> learner;
  if (options.was_supplied("experimental_igl"))
  {
    // Both label and prediction types inherit that of base.
    learner = VW::LEARNER::make_reduction_learner(std::move(data), multi_base, predict_or_learn<true, true>,
        predict_or_learn<false, true>, stack_builder.get_setupfn_name(shared_feature_merger_setup))
                  .set_learn_returns_prediction(base->learn_returns_prediction)
                  .set_persist_metrics(persist)
                  .build();
  }
  else
  {
    // Both label and prediction types inherit that of base.
    learner = VW::LEARNER::make_reduction_learner(std::move(data), multi_base, predict_or_learn<true, false>,
        predict_or_learn<false, false>, stack_builder.get_setupfn_name(shared_feature_merger_setup))
                  .set_learn_returns_prediction(base->learn_returns_prediction)
                  .set_persist_metrics(persist)
                  .build();
  }

  // Note: feature counts may overcount shared features since we don't merge shared example
  //       namespaces in finish_example (too expensive for the accuracy improvement).

  return learner;
}
