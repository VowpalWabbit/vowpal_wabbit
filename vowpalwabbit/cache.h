// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "v_array.h"
#include "io_buf.h"
#include "example.h"
#include "io/logger.h"

namespace VW
{
namespace details
{
struct cache_temp_buffer;

}
}  // namespace VW

char* run_len_decode(char* p, size_t& i);
char* run_len_encode(char* p, size_t i);

void cache_tag(io_buf& cache, const v_array<char>& tag);
void output_byte(io_buf& cache, unsigned char s);
void cache_index(io_buf& cache, unsigned char index, const features& fs, char*& c);
void cache_features(io_buf& cache, const features& fs, uint64_t mask, char*& c);
size_t read_cached_index(io_buf& input, unsigned char& index, char*& c);
size_t read_cached_features(io_buf& input, features& ours, bool& sorted, char*& c);

namespace VW
{
uint32_t convert(size_t number);
// What is written by write_example_to_cache can be read by read_example_from_cache
void write_example_to_cache(io_buf& output, example* ae, label_parser& lbl_parser, uint64_t parse_mask,
    VW::details::cache_temp_buffer& temp_buffer);
int read_example_from_cache(VW::workspace* all, io_buf& buf, v_array<example*>& examples);
}  // namespace VW
