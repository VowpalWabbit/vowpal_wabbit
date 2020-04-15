// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cache.h"
#include "unique_sort.h"
#include "global_data.h"
#include "vw.h"

constexpr size_t int_size = 11;
constexpr size_t char_size = 2;
constexpr size_t neg_1 = 1;
constexpr size_t general = 2;

inline char* run_len_decode(char* p, uint64_t& i)
{
  // read an int 7 bits at a time.
  size_t count = 0;
  while (*p & 128) i = i | ((uint64_t)(*(p++) & 127) << 7 * count++);
  i = i | ((uint64_t)(*(p++)) << 7 * count);
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
  if (cache.buf_read(c, sizeof(tag_size)) < sizeof(tag_size))
    return 0;
  tag_size = *(size_t*)c;
  c += sizeof(tag_size);
  cache.set(c);
  if (cache.buf_read(c, tag_size) < tag_size)
    return 0;

  ae->tag.clear();
  push_many(ae->tag, c, tag_size);
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

int read_cached_features(vw* all, v_array<example*>& examples)
{
  example* ae = examples[0];
  ae->sorted = all->p->sorted_cache;
  io_buf* input = all->p->input;

  size_t total = all->p->lp.read_cached_label(all->p->_shared_data, &ae->l, *input);
  if (total == 0)
    return 0;
  if (read_cached_tag(*input, ae) == 0)
    return 0;
  char* c;
  unsigned char num_indices = 0;
  if (input->buf_read(c, sizeof(num_indices)) < sizeof(num_indices))
    return 0;
  num_indices = *(unsigned char*)c;
  c += sizeof(num_indices);

  all->p->input->set(c);
  for (; num_indices > 0; num_indices--)
  {
    size_t temp;
    unsigned char index = 0;
    if ((temp = input->buf_read(c, sizeof(index) + sizeof(size_t))) < sizeof(index) + sizeof(size_t))
    {
      all->trace_message << "truncated example! " << temp << " " << char_size + sizeof(size_t) << std::endl;
      return 0;
    }

    index = *(unsigned char*)c;
    c += sizeof(index);
    ae->indices.push_back((size_t)index);
    features& ours = ae->feature_space[index];
    size_t storage = *(size_t*)c;
    c += sizeof(size_t);
    all->p->input->set(c);
    total += storage;
    if (input->buf_read(c, storage) < storage)
    {
      all->trace_message << "truncated example! wanted: " << storage << " bytes" << std::endl;
      return 0;
    }

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
        v = ((one_float*)c)->f;
        c += sizeof(float);
      }
      uint64_t diff = i >> 2;
      int64_t s_diff = ZigZagDecode(diff);
      if (s_diff < 0)
        ae->sorted = false;
      i = last + s_diff;
      last = i;
      ours.push_back(v, i);
    }
    all->p->input->set(c);
  }

  return (int)total;
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

void output_features(io_buf& cache, unsigned char index, features& fs, uint64_t mask)
{
  char* c;
  size_t storage = fs.size() * int_size;
  for (feature_value f : fs.values)
    if (f != 1. && f != -1.)
      storage += sizeof(feature_value);

  cache.buf_write(c, sizeof(index) + storage + sizeof(size_t));
  *reinterpret_cast<unsigned char*>(c) = index;
  c += sizeof(index);

  char* storage_size_loc = c;
  c += sizeof(size_t);

  uint64_t last = 0;
  for (features::iterator& f : fs)
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
  *(size_t*)storage_size_loc = c - storage_size_loc - sizeof(size_t);
}

void cache_tag(io_buf& cache, v_array<char> tag)
{
  char* c;
  cache.buf_write(c, sizeof(size_t) + tag.size());
  *(size_t*)c = tag.size();
  c += sizeof(size_t);
  memcpy(c, tag.begin(), tag.size());
  c += tag.size();
  cache.set(c);
}

void cache_features(io_buf& cache, example* ae, uint64_t mask)
{
  cache_tag(cache, ae->tag);
  output_byte(cache, (unsigned char)ae->indices.size());

  for (namespace_index ns : ae->indices) output_features(cache, ns, ae->feature_space[ns], mask);
}

uint32_t VW::convert(size_t number)
{
  if (number > UINT32_MAX)
  {
    THROW("size_t value is out of bounds of uint32_t.")
  }
  return static_cast<uint32_t>(number);
}
