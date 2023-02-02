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
    : _begin(VW::details::calloc_mergable_or_throw<VW::weight>(length << stride_shift))
    , _weight_mask((length << stride_shift) - 1)
    , _stride_shift(stride_shift)
    , _seeded(false)
{
}

VW::dense_parameters::dense_parameters() : _begin(nullptr), _weight_mask(0), _stride_shift(0), _seeded(false) {}
bool VW::dense_parameters::not_null() { return (_weight_mask > 0 && _begin != nullptr); }

void VW::dense_parameters::shallow_copy(const dense_parameters& input)
{
  if (!_seeded) { free(_begin); }
  _begin = input._begin;
  _weight_mask = input._weight_mask;
  _stride_shift = input._stride_shift;
  _seeded = true;
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

VW::dense_parameters::~dense_parameters()
{
  if (_begin != nullptr && !_seeded)  // don't free weight vector if it is shared with another instance
  {
    free(_begin);
    _begin = nullptr;
  }
}