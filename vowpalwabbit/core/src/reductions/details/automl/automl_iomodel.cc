// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../automl_impl.h"
#include "vw/core/confidence_sequence.h"
#include "vw/core/model_utils.h"

namespace VW
{
namespace reductions
{
namespace automl
{
template <typename estimator_impl>
void aml_estimator<estimator_impl>::persist(
    metric_sink& metrics, const std::string& suffix, bool verbose, const std::string& interaction_type)
{
  _estimator.persist(metrics, suffix);
  metrics.set_uint("conf_idx" + suffix, config_index);
  if (verbose)
  {
    metrics.set_string("interactions" + suffix,
        VW::reductions::util::interaction_vec_t_to_string(live_interactions, interaction_type));
  }
}

template struct aml_estimator<VW::confidence_sequence::ConfidenceSequence>;

template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::persist(metric_sink& metrics, bool verbose)
{
  metrics.set_uint("test_county", total_learn_count);
  metrics.set_uint("current_champ", current_champ);
  for (uint64_t live_slot = 0; live_slot < estimators.size(); ++live_slot)
  {
    estimators[live_slot].first.persist(
        metrics, "_amls_" + std::to_string(live_slot), verbose, _config_oracle._interaction_type);
    estimators[live_slot].second.persist(metrics, "_sc_" + std::to_string(live_slot));
    if (verbose)
    {
      auto& elements = _config_oracle.configs[estimators[live_slot].first.config_index].elements;
      metrics.set_string("exclusionc_" + std::to_string(live_slot), VW::reductions::util::elements_to_string(elements));
    }
  }
  metrics.set_uint("total_champ_switches", total_champ_switches);
}

template struct interaction_config_manager<config_oracle<oracle_rand_impl>, VW::confidence_sequence::ConfidenceSequence>;
template struct interaction_config_manager<config_oracle<one_diff_impl>, VW::confidence_sequence::ConfidenceSequence>;
template struct interaction_config_manager<config_oracle<champdupe_impl>, VW::confidence_sequence::ConfidenceSequence>;

}  // namespace automl
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::reductions::automl::ns_based_config& ec)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ec.elements);
  bytes += read_model_field(io, ec.lease);
  bytes += read_model_field(io, ec.state);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::reductions::automl::ns_based_config& config, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  assert(config.conf_type == reductions::automl::config_type::Exclusion);
  bytes += write_model_field(io, config.elements, upstream_name + "_exclusions", text);
  bytes += write_model_field(io, config.lease, upstream_name + "_lease", text);
  bytes += write_model_field(io, config.state, upstream_name + "_state", text);
  return bytes;
}

template <typename estimator_impl>
size_t read_model_field(io_buf& io, VW::reductions::automl::aml_estimator<estimator_impl>& amls)
{
  size_t bytes = 0;
  bytes += read_model_field(io, reinterpret_cast<estimator_impl&>(amls));
  bytes += read_model_field(io, amls.config_index);
  bytes += read_model_field(io, amls.eligible_to_inactivate);
  return bytes;
}

template <typename estimator_impl>
size_t write_model_field(io_buf& io, const VW::reductions::automl::aml_estimator<estimator_impl>& amls,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, reinterpret_cast<const estimator_impl&>(amls), upstream_name, text);
  bytes += write_model_field(io, amls.config_index, upstream_name + "_index", text);
  bytes += write_model_field(io, amls.eligible_to_inactivate, upstream_name + "_eligible_to_inactivate", text);
  return bytes;
}

template <typename config_oracle_impl, typename estimator_impl>
size_t read_model_field(
    io_buf& io, VW::reductions::automl::interaction_config_manager<config_oracle_impl, estimator_impl>& cm)
{
  cm.estimators.clear();
  cm._config_oracle.configs.clear();
  cm.per_live_model_state_double.clear();
  cm.per_live_model_state_uint64.clear();
  size_t bytes = 0;
  uint64_t current_champ = 0;
  bytes += read_model_field(io, cm.total_learn_count);
  bytes += read_model_field(io, current_champ);
  bytes += read_model_field(io, cm._config_oracle.valid_config_size);
  bytes += read_model_field(io, cm.ns_counter);
  bytes += read_model_field(io, cm._config_oracle.configs);
  bytes += read_model_field(io, cm.estimators);
  bytes += read_model_field(io, cm._config_oracle.index_queue);
  bytes += read_model_field(io, cm.per_live_model_state_double);
  bytes += read_model_field(io, cm.per_live_model_state_uint64);
  for (uint64_t live_slot = 0; live_slot < cm.estimators.size(); ++live_slot)
  {
    auto& exclusions = cm._config_oracle.configs[cm.estimators[live_slot].first.config_index];
    assert(cm._config_oracle.configs[cm.estimators[live_slot].first.config_index].conf_type ==
        reductions::automl::config_type::Exclusion);
    auto& interactions = cm.estimators[live_slot].first.live_interactions;
    reductions::automl::ns_based_config::apply_config_to_interactions(
        cm._ccb_on, cm.ns_counter, cm._config_oracle._interaction_type, exclusions, interactions);
  }
  return bytes;
}

template <typename config_oracle_impl, typename estimator_impl>
size_t write_model_field(io_buf& io,
    const VW::reductions::automl::interaction_config_manager<config_oracle_impl, estimator_impl>& cm,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  uint64_t current_champ = 0;
  bytes += write_model_field(io, cm.total_learn_count, upstream_name + "_count", text);
  bytes += write_model_field(io, current_champ, upstream_name + "_champ", text);
  bytes += write_model_field(io, cm._config_oracle.valid_config_size, upstream_name + "_valid_config_size", text);
  bytes += write_model_field(io, cm.ns_counter, upstream_name + "_ns_counter", text);
  bytes += write_model_field(io, cm._config_oracle.configs, upstream_name + "_configs", text);
  bytes += write_model_field(io, cm.estimators, upstream_name + "_estimators", text);
  bytes += write_model_field(io, cm._config_oracle.index_queue, upstream_name + "_index_queue", text);
  bytes += write_model_field(io, cm.per_live_model_state_double, upstream_name + "_per_live_model_state_double", text);
  bytes += write_model_field(io, cm.per_live_model_state_uint64, upstream_name + "_per_live_model_state_uint64", text);
  return bytes;
}

template <typename CMType>
size_t read_model_field(io_buf& io, VW::reductions::automl::automl<CMType>& aml)
{
  size_t bytes = 0;
  bytes += read_model_field(io, aml.current_state);
  bytes += read_model_field(io, *aml.cm);
  return bytes;
}

template size_t read_model_field(io_buf&,
    VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager<
        VW::reductions::automl::config_oracle<VW::reductions::automl::oracle_rand_impl>, VW::confidence_sequence::ConfidenceSequence>>&);
template size_t read_model_field(io_buf&,
    VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager<
        VW::reductions::automl::config_oracle<VW::reductions::automl::one_diff_impl>, VW::confidence_sequence::ConfidenceSequence>>&);
template size_t read_model_field(io_buf&,
    VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager<
        VW::reductions::automl::config_oracle<VW::reductions::automl::champdupe_impl>, VW::confidence_sequence::ConfidenceSequence>>&);

template <typename CMType>
size_t write_model_field(
    io_buf& io, const VW::reductions::automl::automl<CMType>& aml, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, aml.current_state, upstream_name + "_state", text);
  bytes += write_model_field(io, *aml.cm, upstream_name + "_config_manager", text);
  return bytes;
}

template size_t write_model_field(io_buf&,
    const VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager<
        VW::reductions::automl::config_oracle<VW::reductions::automl::oracle_rand_impl>, VW::confidence_sequence::ConfidenceSequence>>&,
    const std::string&, bool);
template size_t write_model_field(io_buf&,
    const VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager<
        VW::reductions::automl::config_oracle<VW::reductions::automl::one_diff_impl>, VW::confidence_sequence::ConfidenceSequence>>&,
    const std::string&, bool);
template size_t write_model_field(io_buf&,
    const VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager<
        VW::reductions::automl::config_oracle<VW::reductions::automl::champdupe_impl>, VW::confidence_sequence::ConfidenceSequence>>&,
    const std::string&, bool);

}  // namespace model_utils

VW::string_view to_string(reductions::automl::automl_state state)
{
  switch (state)
  {
    case reductions::automl::automl_state::Experimenting:
      return "Experimenting";
  }

  assert(false);
  return "unknown";
}

VW::string_view to_string(reductions::automl::config_state state)
{
  switch (state)
  {
    case reductions::automl::config_state::New:
      return "New";
    case reductions::automl::config_state::Live:
      return "Live";
    case reductions::automl::config_state::Inactive:
      return "Inactive";
    case reductions::automl::config_state::Removed:
      return "Removed";
  }

  assert(false);
  return "unknown";
}
}  // namespace VW
