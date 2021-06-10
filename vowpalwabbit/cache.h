// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "v_array.h"
#include "io_buf.h"
#include "example.h"
#include "future_compat.h"

FORCE_INLINE int read_cached_features(vw* all, v_array<example*>& examples);
FORCE_INLINE void cache_tag(io_buf& cache, const v_array<char>& tag);
FORCE_INLINE void cache_features(io_buf& cache, const example* ae, uint64_t mask);
FORCE_INLINE void output_byte(io_buf& cache, unsigned char s);
FORCE_INLINE void output_features(io_buf& cache, unsigned char index, const features& fs, uint64_t mask);

namespace VW
{
// What is written by write_example_to_cache can be read by read_example_from_cache
FORCE_INLINE void write_example_to_cache(io_buf& output, const example* ae, const label_parser& lbl_parser, uint64_t parse_mask);
FORCE_INLINE int read_example_from_cache(
    io_buf& input, example* ae, const label_parser& lbl_parser, bool sorted_cache, shared_data* shared_dat);
}  // namespace VW
