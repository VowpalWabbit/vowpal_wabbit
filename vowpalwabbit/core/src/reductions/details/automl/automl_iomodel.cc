// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../automl_impl.h"
#include "vw/core/model_utils.h"

namespace VW
{
namespace reductions
{
namespace automl
{
void aml_estimator::persist(
    metric_sink& metrics, const std::string& suffix, bool verbose, const std::string& interaction_type)
{
  VW::estimator_config::persist(metrics, suffix);
  metrics.set_uint("conf_idx" + suffix, config_index);
  if (verbose)
  {
    metrics.set_string("interactions" + suffix,
        VW::reductions::util::interaction_vec_t_to_string(live_interactions, interaction_type));
  }
}

template <typename oracle_impl>
void interaction_config_manager<oracle_impl>::persist(metric_sink& metrics, bool verbose)
{
  metrics.set_uint("test_county", total_learn_count);
  metrics.set_uint("current_champ", current_champ);
  for (uint64_t live_slot = 0; live_slot < estimators.size(); ++live_slot)
  {
    estimators[live_slot].first.persist(metrics, "_amls_" + std::to_string(live_slot), verbose, interaction_type);
    estimators[live_slot].second.persist(metrics, "_sc_" + std::to_string(live_slot));
    if (verbose)
    {
      auto& exclusions = configs[estimators[live_slot].first.config_index].exclusions;
      metrics.set_string(
          "exclusionc_" + std::to_string(live_slot), VW::reductions::util::exclusions_to_string(exclusions));
    }
  }
  metrics.set_uint("total_champ_switches", total_champ_switches);
}

template class interaction_config_manager<oracle_rand_impl>;

}  // namespace automl
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::reductions::automl::exclusion_config& ec)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ec.exclusions);
  bytes += read_model_field(io, ec.lease);
  bytes += read_model_field(io, ec.state);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::reductions::automl::exclusion_config& ec, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ec.exclusions, upstream_name + "_exclusions", text);
  bytes += write_model_field(io, ec.lease, upstream_name + "_lease", text);
  bytes += write_model_field(io, ec.state, upstream_name + "_state", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::reductions::automl::aml_estimator& amls)
{
  size_t bytes = 0;
  bytes += read_model_field(io, reinterpret_cast<VW::estimator_config&>(amls));
  bytes += read_model_field(io, amls.config_index);
  bytes += read_model_field(io, amls.eligible_to_inactivate);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::reductions::automl::aml_estimator& amls, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, reinterpret_cast<const VW::estimator_config&>(amls), upstream_name, text);
  bytes += write_model_field(io, amls.config_index, upstream_name + "_index", text);
  bytes += write_model_field(io, amls.eligible_to_inactivate, upstream_name + "_eligible_to_inactivate", text);
  return bytes;
}

template <typename oracle_impl>
size_t read_model_field(io_buf& io, VW::reductions::automl::interaction_config_manager<oracle_impl>& cm)
{
  cm.estimators.clear();
  cm.configs.clear();
  cm.per_live_model_state_double.clear();
  cm.per_live_model_state_uint64.clear();
  size_t bytes = 0;
  uint64_t current_champ = 0;
  bytes += read_model_field(io, cm.total_learn_count);
  bytes += read_model_field(io, current_champ);
  bytes += read_model_field(io, cm._config_oracle.valid_config_size);
  bytes += read_model_field(io, cm.ns_counter);
  bytes += read_model_field(io, cm.configs);
  bytes += read_model_field(io, cm.estimators);
  bytes += read_model_field(io, cm.index_queue);
  bytes += read_model_field(io, cm.per_live_model_state_double);
  bytes += read_model_field(io, cm.per_live_model_state_uint64);
  for (uint64_t live_slot = 0; live_slot < cm.estimators.size(); ++live_slot)
  { gen_interactions(cm.ccb_on, cm.ns_counter, cm.interaction_type, cm.configs, cm.estimators, live_slot); }
  return bytes;
}

template <typename oracle_impl>
size_t write_model_field(io_buf& io, const VW::reductions::automl::interaction_config_manager<oracle_impl>& cm,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  uint64_t current_champ = 0;
  bytes += write_model_field(io, cm.total_learn_count, upstream_name + "_count", text);
  bytes += write_model_field(io, current_champ, upstream_name + "_champ", text);
  bytes += write_model_field(io, cm._config_oracle.valid_config_size, upstream_name + "_valid_config_size", text);
  bytes += write_model_field(io, cm.ns_counter, upstream_name + "_ns_counter", text);
  bytes += write_model_field(io, cm.configs, upstream_name + "_configs", text);
  bytes += write_model_field(io, cm.estimators, upstream_name + "_estimators", text);
  bytes += write_model_field(io, cm.index_queue, upstream_name + "_index_queue", text);
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
    VW::reductions::automl::automl<
        VW::reductions::automl::interaction_config_manager<VW::reductions::automl::oracle_rand_impl>>&);

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
    const VW::reductions::automl::automl<
        VW::reductions::automl::interaction_config_manager<VW::reductions::automl::oracle_rand_impl>>&,
    const std::string&, bool);

}  // namespace model_utils

VW::string_view to_string(reductions::automl::automl_state state)
{
  switch (state)
  {
    case reductions::automl::automl_state::Collecting:
      return "Collecting";
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
