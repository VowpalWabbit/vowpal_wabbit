// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/cache_parser/parse_example_cache.h"

#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/io_buf.h"
#include "vw/core/parser.h"
#include "vw/io/io_adapter.h"

#include <cstdint>
#include <memory>

namespace
{
constexpr size_t INTS_SIZE = 11;
constexpr size_t NEG_ONE = 1;
constexpr size_t GENERAL = 2;
constexpr unsigned char NEWLINE_EXAMPLE_INDICATOR = '1';
constexpr unsigned char NON_NEWLINE_EXAMPLE_INDICATOR = '0';

// Integers are written/read 1 byte at at time (using 7 bits for the number
// representation and the msb used to signal further bytes), with only the
// number of bytes required to represent the number being used. For an
// explanation of how this words see here:
// https://developers.google.com/protocol-buffers/docs/encoding#varints
//
// See the function for writing: `variable_length_int_encode`
inline char* variable_length_int_decode(char* read_head, uint64_t& num)
{
  // read an int 7 bits at a time.
  size_t count = 0;
  while ((*read_head & 128) != 0) { num = num | (static_cast<uint64_t>(*(read_head++) & 127) << 7 * count++); }
  num = num | (static_cast<uint64_t>(*(read_head++)) << 7 * count);
  return read_head;
}

inline char* variable_length_int_encode(char* write_head, uint64_t num)
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
  uint64_t ret = (static_cast<uint64_t>(n) << 1) ^ (n >> 63);
  return ret;
}

class one_float
{
public:
  float f;
}
#ifndef _WIN32
__attribute__((packed))
#endif
;

}  // namespace

size_t VW::parsers::cache::details::read_cached_tag(io_buf& cache, VW::v_array<char>& tag)
{
  char* read_head = nullptr;
  auto tag_size = cache.read_value<size_t>("tag size");

  if (cache.buf_read(read_head, tag_size) < tag_size) { return 0; }

  tag.clear();
  tag.insert(tag.end(), read_head, read_head + tag_size);
  return tag_size + sizeof(tag_size);
}

size_t VW::parsers::cache::details::read_cached_index(io_buf& input, VW::namespace_index& index)
{
  index = input.read_value<VW::namespace_index>("index");
  return sizeof(index);
}

size_t VW::parsers::cache::details::read_cached_features(io_buf& input, features& feats, bool& sorted)
{
  // The example is sorted until we see an example of an unsorted sequence.
  sorted = true;
  size_t total = 0;
  auto storage = input.read_value_and_accumulate_size<size_t>("feature count", total);
  total += storage;
  char* read_head = nullptr;
  if (input.buf_read(read_head, storage) < storage)
  {
    THROW("Ran out of cache while reading example. File may be truncated.");
  }

  char* end = read_head + storage;
  uint64_t last = 0;

  for (; read_head < end;)
  {
    feature_index feat_idx = 0;
    read_head = variable_length_int_decode(read_head, feat_idx);
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

void VW::parsers::cache::details::cache_tag(io_buf& cache, const VW::v_array<char>& tag)
{
  char* write_head = nullptr;
  size_t tag_size = tag.size();
  cache.buf_write(write_head, sizeof(size_t) + tag_size);
  std::memcpy(write_head, &tag_size, sizeof(size_t));
  write_head += sizeof(size_t);
  if (tag_size > 0)
  {
    std::memcpy(write_head, tag.begin(), tag_size);
    write_head += tag_size;
  }
  cache.set(write_head);
}

void VW::parsers::cache::details::cache_index(io_buf& cache, VW::namespace_index index)
{
  cache.write_value<VW::namespace_index>(index);
}

void VW::parsers::cache::details::cache_features(io_buf& cache, const features& feats, uint64_t mask)
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

    if (feat_it.value() == 1.) { write_head = variable_length_int_encode(write_head, diff); }
    else if (feat_it.value() == -1.) { write_head = variable_length_int_encode(write_head, diff | NEG_ONE); }
    else
    {
      write_head = variable_length_int_encode(write_head, diff | GENERAL);
      std::memcpy(write_head, &feat_it.value(), sizeof(feature_value));
      write_head += sizeof(feature_value);
    }
  }

  cache.set(write_head);
  size_t storage_size = write_head - storage_size_loc - sizeof(size_t);
  std::memcpy(storage_size_loc, &storage_size, sizeof(size_t));
}

void VW::parsers::cache::write_example_to_cache(io_buf& output, example* ex_ptr, VW::label_parser& lbl_parser,
    uint64_t parse_mask, VW::parsers::cache::details::cache_temp_buffer& temp_buffer)
{
  temp_buffer.backing_buffer->clear();
  io_buf& temp_cache = temp_buffer.temporary_cache_buffer;
  lbl_parser.cache_label(ex_ptr->l, ex_ptr->ex_reduction_features, temp_cache, "_label", false);
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

  uint64_t example_size = temp_buffer.backing_buffer->size();
  output.write_value(example_size);
  output.bin_write_fixed(temp_buffer.backing_buffer->data(), temp_buffer.backing_buffer->size());
}

int VW::parsers::cache::read_example_from_cache(VW::workspace* all, io_buf& input, VW::multi_ex& examples)
{
  assert(all != nullptr);
  // uint64_t size; TODO: Use to be able to skip cached examples on a read failure.
  char* unused_read_ptr = nullptr;
  // If this read returns 0 bytes, it means that we've reached the end of the cache file in an expected way.
  // (As opposed to being unable to get the next bytes while midway through reading an example)
  if (input.buf_read(unused_read_ptr, sizeof(uint64_t)) < sizeof(uint64_t)) { return 0; }

  all->example_parser->lbl_parser.default_label(examples[0]->l);
  size_t total =
      all->example_parser->lbl_parser.read_cached_label(examples[0]->l, examples[0]->ex_reduction_features, input);
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
