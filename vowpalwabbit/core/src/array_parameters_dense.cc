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

dense_parameters::dense_parameters(size_t length, uint32_t stride_shift)
    : _begin(calloc_mergable_or_throw<VW::weight>(length << stride_shift))
    , _weight_mask((length << stride_shift) - 1)
    , _stride_shift(stride_shift)
    , _seeded(false)
{
}

dense_parameters::dense_parameters() : _begin(nullptr), _weight_mask(0), _stride_shift(0), _seeded(false) {}
bool dense_parameters::not_null() { return (_weight_mask > 0 && _begin != nullptr); }

void dense_parameters::shallow_copy(const dense_parameters& input)
{
  if (!_seeded) { free(_begin); }
  _begin = input._begin;
  _weight_mask = input._weight_mask;
  _stride_shift = input._stride_shift;
  _seeded = true;
}

void dense_parameters::set_zero(size_t offset)
{
  if (not_null())
  {
    for (iterator iter = begin(); iter != end(); ++iter) { (&(*iter))[offset] = 0; }
  }
}

void dense_parameters::move_offsets(const size_t from, const size_t to, const size_t params_per_problem, bool swap)
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
        else
        {
          *iterator_to[stride_offset] = *iterator_from[stride_offset];
        }
      }
    }
  }
}

// ***** NOTE: params_per_problem must be of form 2^n *****
void dense_parameters::clear_offset(const size_t offset, const size_t params_per_problem)
{
  assert(offset < params_per_problem);

  for (auto iterator_clear = begin() + offset; iterator_clear < end(); iterator_clear += params_per_problem)
  {
    assert((iterator_clear.index_without_stride() & (params_per_problem - 1)) == offset);
    for (size_t stride_offset = 0; stride_offset < stride(); stride_offset++)
    {
      if (*iterator_clear[stride_offset] != 0.0f) { *iterator_clear[stride_offset] = 0.0f; }
    }
  }
}

#ifndef _WIN32
#  ifndef DISABLE_SHARED_WEIGHTS
void dense_parameters::share(size_t length)
{
  float* shared_weights = static_cast<float*>(mmap(
      nullptr, (length << _stride_shift) * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  size_t float_count = length << _stride_shift;
  VW::weight* dest = shared_weights;
  memcpy(dest, _begin, float_count * sizeof(float));
  free(_begin);
  _begin = dest;
}
#  endif
#endif

dense_parameters::~dense_parameters()
{
  if (_begin != nullptr && !_seeded)  // don't free weight vector if it is shared with another instance
  {
    free(_begin);
    _begin = nullptr;
  }
}