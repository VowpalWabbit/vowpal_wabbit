// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/metric_sink.h"

#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"

void VW::metric_sink::throw_if_not_overwrite_and_key_exists(const std::string& key, bool overwrite)
{
  if (!overwrite)
  {
    auto key_exists = _keys.count(key) > 0;
    if (key_exists)
    {
      THROW("Key: " << key << " already exists in metrics. Set overwrite to true if this should be overwritten.")
    }
  }
}

void VW::metric_sink::set_uint(const std::string& key, uint64_t value, bool overwrite)
{
  throw_if_not_overwrite_and_key_exists(key, overwrite);
  _int_metrics[key] = value;
  _keys.insert(key);
}

void VW::metric_sink::set_float(const std::string& key, float value, bool overwrite)
{
  throw_if_not_overwrite_and_key_exists(key, overwrite);
  _float_metrics[key] = value;
  _keys.insert(key);
}

void VW::metric_sink::set_string(const std::string& key, const std::string& value, bool overwrite)
{
  throw_if_not_overwrite_and_key_exists(key, overwrite);
  _string_metrics[key] = value;
  _keys.insert(key);
}

void VW::metric_sink::set_bool(const std::string& key, bool value, bool overwrite)
{
  throw_if_not_overwrite_and_key_exists(key, overwrite);
  _bool_metrics[key] = value;
  _keys.insert(key);
}

void VW::metric_sink::set_metric_sink(const std::string& key, VW::metric_sink value, bool overwrite)
{
  throw_if_not_overwrite_and_key_exists(key, overwrite);
  _metric_sink_metrics[key] = std::move(value);
  _keys.insert(key);
}

uint64_t VW::metric_sink::get_uint(const std::string& key) const
{
  auto it = _int_metrics.find(key);
  if (it == _int_metrics.end()) { THROW("Key: " << key << " does not exist in uint metrics. Is the type correct?") }
  return it->second;
}

float VW::metric_sink::get_float(const std::string& key) const
{
  auto it = _float_metrics.find(key);
  if (it == _float_metrics.end()) { THROW("Key: " << key << " does not exist in float metrics. Is the type correct?") }
  return it->second;
}

VW::string_view VW::metric_sink::get_string(const std::string& key) const
{
  auto it = _string_metrics.find(key);
  if (it == _string_metrics.end())
  {
    THROW("Key: " << key << " does not exist in string metrics. Is the type correct?")
  }
  return it->second;
}

bool VW::metric_sink::get_bool(const std::string& key) const
{
  auto it = _bool_metrics.find(key);
  if (it == _bool_metrics.end()) { THROW("Key: " << key << " does not exist in sink metrics. Is the type correct?") }
  return it->second;
}

VW::metric_sink VW::metric_sink::get_metric_sink(const std::string& key) const
{
  auto it = _metric_sink_metrics.find(key);
  if (it == _metric_sink_metrics.end())
  {
    THROW("Key: " << key << " does not exist in bool metrics. Is the type correct?")
  }
  return it->second;
}

void VW::metric_sink::visit(metric_sink_visitor& visitor) const
{
  for (const auto& kv : _int_metrics) { visitor.int_metric(kv.first, kv.second); }
  for (const auto& kv : _float_metrics) { visitor.float_metric(kv.first, kv.second); }
  for (const auto& kv : _string_metrics) { visitor.string_metric(kv.first, kv.second); }
  for (const auto& kv : _bool_metrics) { visitor.bool_metric(kv.first, kv.second); }
  for (const auto& kv : _metric_sink_metrics) { visitor.sink_metric(kv.first, kv.second); }
}
