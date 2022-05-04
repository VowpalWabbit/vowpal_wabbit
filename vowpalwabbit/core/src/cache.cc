// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/cache.h"

#include "vw/core/global_data.h"
#include "vw/core/io_buf.h"
#include "vw/core/parser.h"
#include "vw/core/shared_data.h"
#include "vw/core/unique_sort.h"
#include "vw/core/vw_fwd.h"
#include "vw/io/io_adapter.h"
#include "vw/io/logger.h"

#include <cstdint>
#include <memory>

namespace
{
constexpr size_t INTS_SIZE = 11;
constexpr size_t NEG_ONE = 1;
constexpr size_t GENERAL = 2;
constexpr unsigned char NEWLINE_EXAMPLE_INDICATOR = '1';
constexpr unsigned char NON_NEWLINE_EXAMPLE_INDICATOR = '0';

inline char* run_len_decode(char* read_head, uint64_t& num)
{
  // read an int 7 bits at a time.
  size_t count = 0;
  while ((*read_head & 128) != 0) { num = num | (static_cast<uint64_t>(*(read_head++) & 127) << 7 * count++); }
  num = num | (static_cast<uint64_t>(*(read_head++)) << 7 * count);
  return read_head;
}

inline char* run_len_encode(char* write_head, uint64_t num)
{
  // store an int 7 bits at a time.
  while (num >= 128)
  {
    *(write_head++) = (num & 127) | 128;
    num = num >> 7;
  }
  *(write_head++) = (num & 127);
  return write_head;
}

inline int64_t zig_zag_decode(uint64_t n) { return (n >> 1) ^ -static_cast<int64_t>(n & 1); }

inline uint64_t zig_zag_encode(int64_t n)
{
  uint64_t ret = (n << 1) ^ (n >> 63);
  return ret;
}

struct one_float
{
  float f;
}
#ifndef _WIN32
__attribute__((packed))
#endif
;

}  // namespace

size_t VW::details::read_cached_tag(io_buf& cache, VW::v_array<char>& tag)
{
  char* read_head = nullptr;
  auto tag_size = cache.read_value<size_t>("tag size");

  if (cache.buf_read(read_head, tag_size) < tag_size) { return 0; }

  tag.clear();
  tag.insert(tag.end(), read_head, read_head + tag_size);
  return tag_size + sizeof(tag_size);
}

size_t VW::details::read_cached_index(io_buf& input, VW::namespace_index& index)
{
  index = input.read_value<VW::namespace_index>("index");
  return sizeof(index);
}

size_t VW::details::read_cached_features(io_buf& input, features& feats, bool& sorted)
{
  size_t total = 0;
  auto storage = input.read_value_and_accumulate_size<size_t>("feature count", total);
  total += storage;
  char* read_head = nullptr;
  if (input.buf_read(read_head, storage) < storage)
  { THROW("Ran out of cache while reading example. File may be truncated."); }

  char* end = read_head + storage;
  uint64_t last = 0;

  for (; read_head < end;)
  {
    feature_index feat_idx = 0;
    read_head = run_len_decode(read_head, feat_idx);
    feature_value feat_value = 1.f;
    if ((feat_idx & NEG_ONE) != 0u) { feat_value = -1.; }
    else if ((feat_idx & GENERAL) != 0u)
    {
      feat_value = (reinterpret_cast<one_float*>(read_head))->f;
      read_head += sizeof(float);
    }
    uint64_t diff = feat_idx >> 2;
    int64_t s_diff = zig_zag_decode(diff);
    if (s_diff < 0) { sorted = false; }
    feat_idx = last + s_diff;
    last = feat_idx;
    feats.push_back(feat_value, feat_idx);
  }
  assert(read_head == end);
  input.set(read_head);
  return total;
}

void VW::details::cache_tag(io_buf& cache, const VW::v_array<char>& tag)
{
  char* write_head = nullptr;
  cache.buf_write(write_head, sizeof(size_t) + tag.size());
  *reinterpret_cast<size_t*>(write_head) = tag.size();
  write_head += sizeof(size_t);
  memcpy(write_head, tag.begin(), tag.size());
  write_head += tag.size();
  cache.set(write_head);
}

void VW::details::cache_index(io_buf& cache, VW::namespace_index index)
{
  cache.write_value<VW::namespace_index>(index);
}

void VW::details::cache_features(io_buf& cache, const features& feats, uint64_t mask)
{
  size_t storage = feats.size() * INTS_SIZE;
  for (auto feat : feats.values)
  {
    if (feat != 1. && feat != -1.) { storage += sizeof(feature_value); }
  }

  char* write_head = nullptr;
  cache.buf_write(write_head, storage + sizeof(size_t));

  char* storage_size_loc = write_head;
  write_head += sizeof(size_t);

  uint64_t last = 0;
  for (const auto& feat_it : feats)
  {
    feature_index feat_index = feat_it.index() & mask;
    int64_t s_diff = (feat_index - last);
    uint64_t diff = zig_zag_encode(s_diff) << 2;
    last = feat_index;

    if (feat_it.value() == 1.) { write_head = run_len_encode(write_head, diff); }
    else if (feat_it.value() == -1.)
    {
      write_head = run_len_encode(write_head, diff | NEG_ONE);
    }
    else
    {
      write_head = run_len_encode(write_head, diff | GENERAL);
      memcpy(write_head, &feat_it.value(), sizeof(feature_value));
      write_head += sizeof(feature_value);
    }
  }

  cache.set(write_head);
  *reinterpret_cast<size_t*>(storage_size_loc) = write_head - storage_size_loc - sizeof(size_t);
}

void VW::write_example_to_cache(io_buf& output, example* ex_ptr, const VW::label_parser& lbl_parser,
    uint64_t parse_mask, VW::details::cache_temp_buffer& temp_buffer)
{
  temp_buffer._backing_buffer->clear();
  io_buf& temp_cache = temp_buffer._temporary_cache_buffer;
  lbl_parser.cache_label(ex_ptr->l, ex_ptr->_reduction_features, temp_cache, "_label", false);
  details::cache_tag(temp_cache, ex_ptr->tag);
  temp_cache.write_value<unsigned char>(ex_ptr->is_newline ? NEWLINE_EXAMPLE_INDICATOR : NON_NEWLINE_EXAMPLE_INDICATOR);
  assert(ex_ptr->indices.size() < 256);
  temp_cache.write_value<unsigned char>(static_cast<unsigned char>(ex_ptr->indices.size()));
  for (VW::namespace_index ns_idx : ex_ptr->indices)
  {
    details::cache_index(temp_cache, ns_idx);
    details::cache_features(temp_cache, ex_ptr->feature_space[ns_idx], parse_mask);
  }
  temp_cache.flush();

  uint64_t example_size = temp_buffer._backing_buffer->size();
  output.write_value(example_size);
  output.bin_write_fixed(temp_buffer._backing_buffer->data(), temp_buffer._backing_buffer->size());
}

int VW::read_example_from_cache(VW::workspace* all, io_buf& input, v_array<example*>& examples)
{
  assert(all != nullptr);
  // uint64_t size; TODO: Use to be able to skip cached examples on a read failure.
  char* unused_read_ptr = nullptr;
  // If this read returns 0 bytes, it means that we've reached the end of the cache file in an expected way.
  // (As opposed to being unable to get the next bytes while midway through reading an example)
  if (input.buf_read(unused_read_ptr, sizeof(uint64_t)) < sizeof(uint64_t)) { return 0; }

  examples[0]->sorted = all->example_parser->sorted_cache;
  size_t total =
      all->example_parser->lbl_parser.read_cached_label(examples[0]->l, examples[0]->_reduction_features, input);
  if (total == 0) { THROW("Ran out of cache while reading example. File may be truncated."); }

  size_t tag_size = details::read_cached_tag(input, examples[0]->tag);
  if (tag_size == 0) { THROW("Ran out of cache while reading example. File may be truncated."); }
  total += tag_size;
  examples[0]->is_newline =
      (input.read_value_and_accumulate_size<unsigned char>("newline_indicator", total) == NEWLINE_EXAMPLE_INDICATOR);

  // read indices
  auto num_indices = input.read_value_and_accumulate_size<unsigned char>("num_indices", total);
  for (; num_indices > 0; num_indices--)
  {
    unsigned char index = 0;
    total += details::read_cached_index(input, index);
    examples[0]->indices.push_back(static_cast<size_t>(index));
    total += details::read_cached_features(input, examples[0]->feature_space[index], examples[0]->sorted);
  }

  return static_cast<int>(total);
}