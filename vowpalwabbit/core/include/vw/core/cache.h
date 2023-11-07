// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/cache_parser/parse_example_cache.h"
#include "vw/common/future_compat.h"

namespace VW
{
VW_DEPRECATED("VW::write_example_to_cache moved to VW::parsers::cache::write_example_to_cache")
inline void write_example_to_cache(io_buf& output, VW::example* ex_ptr, VW::label_parser& lbl_parser,
    uint64_t parse_mask, VW::parsers::cache::details::cache_temp_buffer& temp_buffer)
{
  return VW::parsers::cache::write_example_to_cache(output, ex_ptr, lbl_parser, parse_mask, temp_buffer);
}
VW_DEPRECATED("VW::read_example_from_cache moved to VW::parsers::cache::read_example_from_cache")
inline int read_example_from_cache(VW::workspace* all, io_buf& input, VW::multi_ex& examples)
{
  return VW::parsers::cache::read_example_from_cache(all, input, examples);
}
}  // namespace VW
