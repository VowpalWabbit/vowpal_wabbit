#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.=

#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "vw_string_view.h"

namespace VW
{

struct metric_sink_visitor
{
  virtual ~metric_sink_visitor() = default;
  virtual void int_metric(const std::string& key, uint64_t value) = 0;
  virtual void float_metric(const std::string& key, float value) = 0;
  virtual void string_metric(const std::string& key, const std::string& value) = 0;
  virtual void bool_metric(const std::string& key, bool value) = 0;
};

struct metric_sink
{
  void set(const std::string& key, uint64_t value, bool overwrite = false);
  void set(const std::string& key, float value, bool overwrite = false);
  void set(const std::string& key, const std::string& value, bool overwrite = false);
  void set(const std::string& key, bool value, bool overwrite = false);

  void visit(metric_sink_visitor& visitor) const;

  uint64_t get_uint(const std::string& key) const;
  float get_float(const std::string& key) const;
  VW::string_view get_string(const std::string& key) const;
  bool get_bool(const std::string& key) const;

private:
  void throw_if_not_overwrite_and_key_exists(const std::string& key, bool overwrite);
  std::set<std::string> _keys;
  std::map<std::string, uint64_t> _int_metrics;
  std::map<std::string, float> _float_metrics;
  std::map<std::string, std::string> _string_metrics;
  std::map<std::string, bool> _bool_metrics;
};
}  // namespace VW
