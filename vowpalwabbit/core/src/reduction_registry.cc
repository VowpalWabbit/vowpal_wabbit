// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reduction_registry.h"

#include "vw/common/vw_throw.h"

std::map<std::string, VW::reduction_setup_fn>& VW::reduction_registry::map_instance()
{
    static std::map<std::string, VW::reduction_setup_fn> static_map_instance;
    return static_map_instance;
}

void  VW::reduction_registry::register_reduction(const std::string& name, VW::reduction_setup_fn f)
{
  auto& map = map_instance();
  if (map.find(name) != map.end())
  {
    THROW("Reduction with name '" << name << "' already registered");
  }
  map[name] = f;
}
