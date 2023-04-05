// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/setup_base.h"

#include <map>
#include <string>

namespace VW
{

class reduction_registry
{
public:
  static void register_reduction(const std::string&, VW::reduction_setup_fn);

  static const std::map<std::string, VW::reduction_setup_fn>& name_setup_mapping() { return map_instance(); }

  static VW::reduction_setup_fn get(const std::string& key) { return map_instance().at(key); }

private:
  static std::map<std::string, VW::reduction_setup_fn>& map_instance();
};

class reduction_registry_entry
{
public:
  reduction_registry_entry(const std::string& name, VW::reduction_setup_fn fn)
  {
    reduction_registry::register_reduction(name, fn);
  }
};
}  // namespace VW
