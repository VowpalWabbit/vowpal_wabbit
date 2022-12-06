// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/io_buf.h"
#include "vw/core/multi_ex.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"
#include "vw/io/io_adapter.h"

namespace VW
{
namespace parsers
{
namespace cache
{
namespace details
{

class cache_temp_buffer
{
public:
  std::shared_ptr<std::vector<char>> backing_buffer;
  io_buf temporary_cache_buffer;
  cache_temp_buffer()
  {
    backing_buffer = std::make_shared<std::vector<char>>();
    temporary_cache_buffer.add_file(VW::io::create_vector_writer(backing_buffer));
  }
};

void cache_tag(io_buf& cache, const VW::v_array<char>& tag);
void cache_index(io_buf& cache, VW::namespace_index index);
void cache_features(io_buf& cache, const features& feats, uint64_t mask);
size_t read_cached_tag(io_buf& cache, VW::v_array<char>& tag);
size_t read_cached_index(io_buf& input, VW::namespace_index& index);
size_t read_cached_features(io_buf& input, features& feats, bool& sorted);
}  // namespace details

// What is written by write_example_to_cache can be read by read_example_from_cache
void write_example_to_cache(io_buf& output, VW::example* ex_ptr, VW::label_parser& lbl_parser, uint64_t parse_mask,
    details::cache_temp_buffer& temp_buffer);
int read_example_from_cache(VW::workspace* all, io_buf& input, VW::multi_ex& examples);

}  // namespace cache
}  // namespace parsers
}  // namespace VW
