// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/option.h"

#include <regex>

void VW::config::base_option::set_tags(std::vector<std::string> tags)
{
  std::sort(tags.begin(), tags.end());
  auto last = std::unique(tags.begin(), tags.end());
  if (last != tags.end())
  {
    std::stringstream ss;
    ss << "Duplicate tags found in option: " << m_name << ". Tags: ";
    for (auto it = tags.begin(); it != last; ++it) { ss << *it << ", "; }
    ss << *last;
    THROW(ss.str());
  }
  for (const auto& tag : tags)
  {
    if (!std::regex_match(tag, std::regex("^[a-z_]+$")))
    {
      std::stringstream ss;
      ss << "Invalid tag found in option: " << m_name << ". Tag: " << tag
         << ". Tags must be lowercase and contain only letters and underscores.";
      THROW(ss.str());
    }
  }
  _tags = std::move(tags);
}
