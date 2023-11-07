// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "options.h"
#include "vw/common/future_compat.h"

#include <set>
#include <string>
#include <vector>

namespace VW
{
namespace config
{
class options_name_extractor : public options_i
{
public:
  std::string generated_name;
  std::set<std::string> m_added_help_group_names;

  void internal_add_and_parse(const option_group_definition& group) override;
  VW_ATTR(nodiscard) bool was_supplied(const std::string&) const override;
  std::vector<std::string> check_unregistered() override;
  void insert(const std::string&, const std::string&) override;
  void replace(const std::string&, const std::string&) override;
  VW_ATTR(nodiscard) std::vector<std::string> get_positional_tokens() const override;
};

}  // namespace config
}  // namespace VW
