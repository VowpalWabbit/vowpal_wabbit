// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/model_utils.h"
#include "vw/core/reductions/automl/automl_impl.h"

namespace VW
{
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

size_t read_model_field(io_buf& io, VW::reductions::automl::interaction_config_manager& cm)
{
  cm.estimators.clear();
  cm.configs.clear();
  cm.per_live_model_state_double.clear();
  cm.per_live_model_state_uint64.clear();
  size_t bytes = 0;
  bytes += read_model_field(io, cm.total_learn_count);
  bytes += read_model_field(io, cm.current_champ);
  bytes += read_model_field(io, cm.valid_config_size);
  bytes += read_model_field(io, cm.ns_counter);
  bytes += read_model_field(io, cm.configs);
  bytes += read_model_field(io, cm.estimators);
  bytes += read_model_field(io, cm.index_queue);
  bytes += read_model_field(io, cm.per_live_model_state_double);
  bytes += read_model_field(io, cm.per_live_model_state_uint64);
  for (uint64_t live_slot = 0; live_slot < cm.estimators.size(); ++live_slot) { cm.gen_interactions(live_slot); }
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::automl::interaction_config_manager& cm,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, cm.total_learn_count, upstream_name + "_count", text);
  bytes += write_model_field(io, cm.current_champ, upstream_name + "_champ", text);
  bytes += write_model_field(io, cm.valid_config_size, upstream_name + "_valid_config_size", text);
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

template <typename CMType>
size_t write_model_field(
    io_buf& io, const VW::reductions::automl::automl<CMType>& aml, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, aml.current_state, upstream_name + "_state", text);
  bytes += write_model_field(io, *aml.cm, upstream_name + "_config_manager", text);
  return bytes;
}

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
