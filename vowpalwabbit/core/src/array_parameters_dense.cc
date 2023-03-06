// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/array_parameters_dense.h"

#include "vw/core/memory.h"

#include <cassert>
#include <cstdint>

#ifndef _WIN32
#  include <sys/mman.h>
#endif

// It appears that on OSX MAP_ANONYMOUS is mapped to MAP_ANON
// https://github.com/leftmike/foment/issues/4
#ifdef __APPLE__
#  define MAP_ANONYMOUS MAP_ANON
#endif

VW::dense_parameters::dense_parameters(size_t length, uint32_t stride_shift)
    // memory allocated by calloc should be freed by C free()
    : _begin(VW::details::calloc_mergable_or_throw<VW::weight>(length << stride_shift), free)
    , _weight_mask((length << stride_shift) - 1)
    , _stride_shift(stride_shift)
{
}

VW::dense_parameters::dense_parameters() : _begin(nullptr), _weight_mask(0), _stride_shift(0) {}

VW::dense_parameters& VW::dense_parameters::operator=(dense_parameters&& other) noexcept
{
  _begin = std::move(other._begin);
  _weight_mask = other._weight_mask;
  _stride_shift = other._stride_shift;
  return *this;
}

VW::dense_parameters::dense_parameters(dense_parameters&& other) noexcept
{
  _begin = std::move(other._begin);
  _weight_mask = other._weight_mask;
  _stride_shift = other._stride_shift;
}
bool VW::dense_parameters::not_null() { return (_weight_mask > 0 && _begin != nullptr); }

VW::dense_parameters VW::dense_parameters::shallow_copy(const dense_parameters& input)
{
  dense_parameters return_val;
  return_val._begin = input._begin;
  return_val._weight_mask = input._weight_mask;
  return_val._stride_shift = input._stride_shift;
  return return_val;
}

VW::dense_parameters VW::dense_parameters::deep_copy(const dense_parameters& input)
{
  dense_parameters return_val;
  auto length = input._weight_mask + 1;
  return_val._begin.reset(VW::details::calloc_mergable_or_throw<VW::weight>(length), free);
  return_val._weight_mask = input._weight_mask;
  return_val._stride_shift = input._stride_shift;
  std::memcpy(return_val._begin.get(), input._begin.get(), length * sizeof(VW::weight));
  return return_val;
}

void VW::dense_parameters::set_zero(size_t offset)
{
  if (not_null())
  {
    for (iterator iter = begin(); iter != end(); ++iter) { (&(*iter))[offset] = 0; }
  }
}

#ifndef _WIN32
#  ifndef DISABLE_SHARED_WEIGHTS
void VW::dense_parameters::share(size_t length)
{
  VW::weight* shared_weights = static_cast<VW::weight*>(mmap(nullptr, (length << _stride_shift) * sizeof(VW::weight),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  size_t float_count = length << _stride_shift;
  std::shared_ptr<VW::weight> dest(
      shared_weights, [shared_weights, float_count](void*) { munmap(shared_weights, float_count); });
  memcpy(dest.get(), _begin.get(), float_count * sizeof(VW::weight));
  _begin = dest;
}
#  endif
#endif
