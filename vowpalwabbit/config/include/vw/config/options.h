// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "option.h"
#include "option_group_definition.h"
#include "vw/common/future_compat.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace VW
{
namespace config
{
class options_i
{
public:
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
  VW_ATTR(nodiscard)
  typed_option<T>& get_typed_option(const std::string& key)
  {
    return dynamic_cast<typed_option<T>&>(*get_option(key));
  }

  template <typename T>
  VW_ATTR(nodiscard)
  const typed_option<T>& get_typed_option(const std::string& key) const
  {
    return dynamic_cast<const typed_option<T>&>(*get_option(key));
  }

  virtual void internal_add_and_parse(const option_group_definition& group) = 0;
  VW_ATTR(nodiscard) virtual bool was_supplied(const std::string& key) const = 0;
  virtual void insert(const std::string& key, const std::string& value) = 0;
  virtual void replace(const std::string& key, const std::string& value) = 0;
  VW_ATTR(nodiscard) virtual std::vector<std::string> get_positional_tokens() const { return {}; }
  /**
   * @brief Check for unregistered options and validate input. Throws if there
   * is an error. Returns a vector of warning strings if there are warnings produced.
   */
  VW_ATTR(nodiscard) virtual std::vector<std::string> check_unregistered() = 0;
  virtual ~options_i() = default;

  static constexpr const char* DEFAULT_TINT = "general";

protected:
  // Collection that tracks for now
  // setup_function_id (str) -> list of option_group_definition
  std::map<std::string, std::vector<option_group_definition>> _option_group_dic;
  std::vector<option_group_definition> _option_group_definitions;
  std::string _current_reduction_tint = DEFAULT_TINT;
  std::map<std::string, std::shared_ptr<base_option>> _options;
  std::map<char, std::shared_ptr<base_option>> _short_options;
};
}  // namespace config
}  // namespace VW
