// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cache.h"
#include <cstdint>
#include <memory>
#include "io/io_adapter.h"
#include "unique_sort.h"
#include "global_data.h"
#include "shared_data.h"
#include "vw.h"
#include "io/logger.h"

constexpr size_t int_size = 11;
constexpr size_t neg_1 = 1;
constexpr size_t general = 2;
constexpr unsigned char newline_example = '1';
constexpr unsigned char non_newline_example = '0';

inline char* run_len_decode(char* p, uint64_t& i)
{
  // read an int 7 bits at a time.
  size_t count = 0;
  while (*p & 128) i = i | (static_cast<uint64_t>(*(p++) & 127) << 7 * count++);
  i = i | (static_cast<uint64_t>(*(p++)) << 7 * count);
  return p;
}

inline char* run_len_encode(char* p, uint64_t i)
{
  // store an int 7 bits at a time.
  while (i >= 128)
  {
    *(p++) = (i & 127) | 128;
    i = i >> 7;
  }
  *(p++) = (i & 127);
  return p;
}

inline int64_t ZigZagDecode(uint64_t n) { return (n >> 1) ^ -static_cast<int64_t>(n & 1); }

size_t read_cached_tag(io_buf& cache, example* ae)
{
  char* c;
  size_t tag_size;
  if (cache.buf_read(c, sizeof(tag_size)) < sizeof(tag_size)) return 0;
  tag_size = *reinterpret_cast<size_t*>(c);
  c += sizeof(tag_size);
  cache.set(c);
  if (cache.buf_read(c, tag_size) < tag_size) return 0;

  ae->tag.clear();
  ae->tag.insert(ae->tag.end(), c, c + tag_size);
  return tag_size + sizeof(tag_size);
}

struct one_float
{
  float f;
}
#ifndef _WIN32
__attribute__((packed))
#endif
;

void VW::write_example_to_cache(io_buf& output, example* ae, label_parser& lbl_parser, uint64_t parse_mask,
    VW::details::cache_temp_buffer& temp_buffer)
{
  temp_buffer._backing_buffer->clear();
  io_buf& temp_cache = temp_buffer._temporary_cache_buffer;
  lbl_parser.cache_label(ae->l, ae->_reduction_features, temp_cache, "_label", false);
  cache_tag(temp_cache, ae->tag);
  temp_cache.write_value<unsigned char>(ae->is_newline ? newline_example : non_newline_example);
  assert(ae->indices.size() < 256);
  temp_cache.write_value<unsigned char>(static_cast<unsigned char>(ae->indices.size()));
  for (namespace_index ns : ae->indices)
  {
    char* c;
    cache_index(temp_cache, ns, ae->feature_space[ns], c);
    cache_features(temp_cache, ae->feature_space[ns], parse_mask, c);
  }
  temp_cache.flush();

  uint64_t example_size = temp_buffer._backing_buffer->size();
  output.write_value(example_size);
  output.bin_write_fixed(temp_buffer._backing_buffer->data(), temp_buffer._backing_buffer->size());
}

size_t read_cached_index(io_buf& input, unsigned char& index, char*& c, VW::io::logger& logger)
{
  size_t temp;
  if ((temp = input.buf_read(c, sizeof(index) + sizeof(size_t))) < sizeof(index) + sizeof(size_t))
  { THROW("Ran out of cache while reading example. File may be truncated."); }
  index = *reinterpret_cast<unsigned char*>(c);
  c += sizeof(index);
  return sizeof(index);
}

size_t read_cached_features(io_buf& input, features& ours, bool& sorted, char*& c, VW::io::logger& logger)
{
  size_t total = 0;
  size_t storage = *reinterpret_cast<size_t*>(c);
  c += sizeof(size_t);
  input.set(c);
  total += storage;
  if (input.buf_read(c, storage) < storage) { THROW("Ran out of cache while reading example. File may be truncated."); }

  char* end = c + storage;
  uint64_t last = 0;

  for (; c != end;)
  {
    feature_index i = 0;
    c = run_len_decode(c, i);
    feature_value v = 1.f;
    if (i & neg_1)
      v = -1.;
    else if (i & general)
    {
      v = (reinterpret_cast<one_float*>(c))->f;
      c += sizeof(float);
    }
    uint64_t diff = i >> 2;
    int64_t s_diff = ZigZagDecode(diff);
    if (s_diff < 0) { sorted = false; }
    i = last + s_diff;
    last = i;
    ours.push_back(v, i);
  }
  input.set(c);
  return total;
}

int VW::read_example_from_cache(VW::workspace* all, io_buf& input, v_array<example*>& examples)
{
  assert(all != nullptr);
  // uint64_t size; TODO: Use to be able to skip cached examples on a read failure.
  char* read_ptr;
  // If this read returns 0 bytes, it means that we've reached the end of the cache file in an expected way.
  // (As opposed to being unable to get the next bytes while midway through reading an example)
  if (input.buf_read(read_ptr, sizeof(uint64_t)) < sizeof(uint64_t)) { return 0; }

  examples[0]->sorted = all->example_parser->sorted_cache;
  size_t total =
      all->example_parser->lbl_parser.read_cached_label(examples[0]->l, examples[0]->_reduction_features, input);
  if (total == 0) { return 0; }
  if (read_cached_tag(input, examples[0]) == 0) { return 0; }
  examples[0]->is_newline = input.read_value<unsigned char>("newline_indicator") == newline_example;

  // read indices
  unsigned char num_indices = input.read_value<unsigned char>("num_indices");
  char* c;
  for (; num_indices > 0; num_indices--)
  {
    unsigned char index = 0;
    total += read_cached_index(input, index, c, all->logger);
    examples[0]->indices.push_back(static_cast<size_t>(index));
    total += read_cached_features(input, examples[0]->feature_space[index], examples[0]->sorted, c, all->logger);
  }

  return static_cast<int>(total);
}

inline uint64_t ZigZagEncode(int64_t n)
{
  uint64_t ret = (n << 1) ^ (n >> 63);
  return ret;
}

void output_byte(io_buf& cache, unsigned char s)
{
  char* c;

  cache.buf_write(c, 1);
  *(c++) = s;
  cache.set(c);
}

void cache_index(io_buf& cache, unsigned char index, const features& fs, char*& c)
{
  size_t storage = fs.size() * int_size;
  for (feature_value f : fs.values)
    if (f != 1. && f != -1.) storage += sizeof(feature_value);

  cache.buf_write(c, sizeof(index) + storage + sizeof(size_t));
  *reinterpret_cast<unsigned char*>(c) = index;
  c += sizeof(index);
}

void cache_features(io_buf& cache, const features& fs, uint64_t mask, char*& c)
{
  char* storage_size_loc = c;
  c += sizeof(size_t);

  uint64_t last = 0;
  for (const auto& f : fs)
  {
    feature_index fi = f.index() & mask;
    int64_t s_diff = (fi - last);
    uint64_t diff = ZigZagEncode(s_diff) << 2;
    last = fi;

    if (f.value() == 1.)
      c = run_len_encode(c, diff);
    else if (f.value() == -1.)
      c = run_len_encode(c, diff | neg_1);
    else
    {
      c = run_len_encode(c, diff | general);
      memcpy(c, &f.value(), sizeof(feature_value));
      c += sizeof(feature_value);
    }
  }

  cache.set(c);
  *reinterpret_cast<size_t*>(storage_size_loc) = c - storage_size_loc - sizeof(size_t);
}

void cache_tag(io_buf& cache, const v_array<char>& tag)
{
  char* c;
  cache.buf_write(c, sizeof(size_t) + tag.size());
  *reinterpret_cast<size_t*>(c) = tag.size();
  c += sizeof(size_t);
  memcpy(c, tag.begin(), tag.size());
  c += tag.size();
  cache.set(c);
}

uint32_t VW::convert(size_t number)
{
  if (number > UINT32_MAX) { THROW("size_t value is out of bounds of uint32_t.") }
  return static_cast<uint32_t>(number);
}
