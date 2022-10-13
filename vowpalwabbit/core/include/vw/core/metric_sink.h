#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.=

#include "vw/common/string_view.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>

namespace VW
{
class metric_sink;
class metric_sink_visitor
{
public:
  virtual ~metric_sink_visitor() = default;
  virtual void int_metric(const std::string& key, uint64_t value) = 0;
  virtual void float_metric(const std::string& key, float value) = 0;
  virtual void string_metric(const std::string& key, const std::string& value) = 0;
  virtual void bool_metric(const std::string& key, bool value) = 0;
  virtual void sink_metric(const std::string& key, const metric_sink& value) = 0;
};

class metric_sink
{
public:
  void set_uint(const std::string& key, uint64_t value, bool overwrite = false);
  void set_float(const std::string& key, float value, bool overwrite = false);
  void set_string(const std::string& key, const std::string& value, bool overwrite = false);
  void set_bool(const std::string& key, bool value, bool overwrite = false);
  void set_metric_sink(const std::string& key, metric_sink value, bool overwrite = false);

  uint64_t get_uint(const std::string& key) const;
  float get_float(const std::string& key) const;
  VW::string_view get_string(const std::string& key) const;
  bool get_bool(const std::string& key) const;
  metric_sink get_metric_sink(const std::string& key) const;

  void visit(metric_sink_visitor& visitor) const;

private:
  void throw_if_not_overwrite_and_key_exists(const std::string& key, bool overwrite);
  std::set<std::string> _keys;
  std::map<std::string, uint64_t> _int_metrics;
  std::map<std::string, float> _float_metrics;
  std::map<std::string, std::string> _string_metrics;
  std::map<std::string, bool> _bool_metrics;
  std::map<std::string, metric_sink> _metric_sink_metrics;
};
}  // namespace VW
