// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/label_dictionary.h"

#include "vw/core/example.h"

void VW::details::add_example_namespace_from_memory(const label_feature_map& lfm, VW::example& ec, size_t lab)
{
  auto res_iter = lfm.find(lab);
  if (res_iter == lfm.end()) { return; }
  VW::add_example_namespace(ec, static_cast<VW::namespace_index>('l'), res_iter->second);
}

void VW::details::del_example_namespace_from_memory(const label_feature_map& lfm, VW::example& ec, size_t lab)
{
  auto res_iter = lfm.find(lab);
  if (res_iter == lfm.end()) { return; }
  VW::del_example_namespace(ec, static_cast<VW::namespace_index>('l'), res_iter->second);
}
