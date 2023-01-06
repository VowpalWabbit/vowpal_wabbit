// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/array_parameters_sparse.h"

#include "vw/common/vw_exception.h"
#include "vw/core/memory.h"

#include <cstddef>
#include <functional>
#include <unordered_map>

VW::weight* VW::sparse_parameters::get_or_default_and_get(size_t i) const
{
  uint64_t index = i & _weight_mask;
  auto iter = _map.find(index);
  if (iter == _map.end())
  {
    _map.insert(std::make_pair(index, VW::details::calloc_mergable_or_throw<VW::weight>(stride())));
    iter = _map.find(index);
    if (_default_func != nullptr) { _default_func(iter->second, index); }
  }
  return iter->second;
}

VW::sparse_parameters::sparse_parameters(size_t length, uint32_t stride_shift)
    : _weight_mask((length << stride_shift) - 1)
    , _stride_shift(stride_shift)
    , _seeded(false)
    , _delete(false)
    , _default_func(nullptr)
{
}

VW::sparse_parameters::sparse_parameters()
    : _weight_mask(0), _stride_shift(0), _seeded(false), _delete(false), _default_func(nullptr)
{
}

VW::sparse_parameters::~sparse_parameters()
{
  if (!_delete && !_seeded)  // don't free weight vector if it is shared with another instance
  {
    for (auto& iter : _map)
    {
      auto* weight_ptr = iter.second;
      free(weight_ptr);
    }
    _map.clear();
    _delete = true;
  }
}

void VW::sparse_parameters::shallow_copy(const sparse_parameters& input)
{
  // TODO: this is level-1 copy (VW::weight* are stilled shared)
  if (!_seeded)
  {
    for (auto& iter : _map)
    {
      auto* weight_ptr = iter.second;
      free(weight_ptr);
    }
  }
  _map = input._map;
  _weight_mask = input._weight_mask;
  _stride_shift = input._stride_shift;
  _seeded = true;
}

void VW::sparse_parameters::set_zero(size_t offset)
{
  for (auto& iter : _map) { (&(*(iter.second)))[offset] = 0; }
}
#ifndef _WIN32
void VW::sparse_parameters::share(size_t /* length */) { THROW_OR_RETURN("Operation not supported on Windows"); }
#endif
