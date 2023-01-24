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
bool VW::dense_parameters::not_null() { return (_weight_mask > 0 && _begin != nullptr); }

void VW::dense_parameters::shallow_copy(const dense_parameters& input)
{
  _begin = input._begin;
  _weight_mask = input._weight_mask;
  _stride_shift = input._stride_shift;
}

void VW::dense_parameters::set_zero(size_t offset)
{
  if (not_null())
  {
    for (iterator iter = begin(); iter != end(); ++iter) { (&(*iter))[offset] = 0; }
  }
}

void VW::dense_parameters::move_offsets(const size_t from, const size_t to, const size_t params_per_problem, bool swap)
{
  assert(from < params_per_problem);
  assert(to < params_per_problem);

  auto iterator_from = begin() + from;
  auto iterator_to = begin() + to;

  for (; iterator_from < end(); iterator_from += params_per_problem, iterator_to += params_per_problem)
  {
    assert((iterator_to.index_without_stride() & (params_per_problem - 1)) == to);
    assert((iterator_from.index_without_stride() & (params_per_problem - 1)) == from);

    for (size_t stride_offset = 0; stride_offset < stride(); stride_offset++)
    {
      if (*iterator_to[stride_offset] != *iterator_from[stride_offset])
      {
        if (swap) { std::swap(*iterator_to[stride_offset], *iterator_from[stride_offset]); }
        else { *iterator_to[stride_offset] = *iterator_from[stride_offset]; }
      }
    }
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
