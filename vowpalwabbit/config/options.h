// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "config/option.h"
#include "config/option_group_definition.h"

#include "future_compat.h"
#include "io/logger.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace VW
{
namespace config
{

struct options_i
{
  void add_and_parse(const option_group_definition& group);
  bool add_parse_and_check_necessary(const option_group_definition& group);

  void tint(const std::string& reduction_name);
  void reset_tint();
  VW_ATTR(nodiscard) std::vector<std::shared_ptr<base_option>> get_all_options();
  VW_ATTR(nodiscard) std::vector<std::shared_ptr<const base_option>> get_all_options() const;
  VW_ATTR(nodiscard) std::shared_ptr<base_option> get_option(const std::string& key);
  VW_ATTR(nodiscard) std::shared_ptr<const base_option> get_option(const std::string& key) const;
  VW_ATTR(nodiscard) std::map<std::string, std::vector<option_group_definition>> get_collection_of_options() const;
  VW_ATTR(nodiscard) const std::vector<option_group_definition>& get_all_option_group_definitions() const;

  template <typename T>
  VW_ATTR(nodiscard) typed_option<T>& get_typed_option(const std::string& key)
  {
    return dynamic_cast<typed_option<T>&>(*get_option(key));
  }

  template <typename T>
  VW_ATTR(nodiscard) const typed_option<T>& get_typed_option(const std::string& key) const
  {
    return dynamic_cast<const typed_option<T>&>(*get_option(key));
  }

  virtual void internal_add_and_parse(const option_group_definition& group) = 0;
  VW_ATTR(nodiscard) virtual bool was_supplied(const std::string& key) const = 0;
  virtual void insert(const std::string& key, const std::string& value) = 0;
  virtual void replace(const std::string& key, const std::string& value) = 0;
  VW_ATTR(nodiscard) virtual std::vector<std::string> get_positional_tokens() const { return {}; }
  VW_ATTR(nodiscard) virtual const std::set<std::string>& get_supplied_options() const = 0;
  // Will throw if any options were supplied that do not having a matching argument specification.
  virtual void check_unregistered(VW::io::logger& logger) = 0;
  virtual ~options_i() = default;

  static constexpr const char* m_default_tint = "general";
protected:

  // Collection that tracks for now
  // setup_function_id (str) -> list of option_group_definition
  std::map<std::string, std::vector<option_group_definition>> m_option_group_dic;
  std::vector<option_group_definition> m_option_group_definitions;
  std::string m_current_reduction_tint = m_default_tint;
  std::map<std::string, std::shared_ptr<base_option>> m_options;
};
}  // namespace config
}  // namespace VW
