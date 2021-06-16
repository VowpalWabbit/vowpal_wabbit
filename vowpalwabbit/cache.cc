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
constexpr unsigned char newline_example = '1';
constexpr unsigned char non_newline_example = '0';

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
  if (cache.buf_read(c, sizeof(tag_size)) < sizeof(tag_size)) return 0;
  tag_size = *(size_t*)c;
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

size_t read_cached_feature(vw *all, std::vector<char>& line, size_t&)
{ 
  std::vector<char> *line_ptr = all->example_parser->io_lines.pop();
  if(line_ptr != nullptr) {
    line = std::move(*line_ptr);
  }

  delete line_ptr;

  return line.size();
}


void notify_examples_cache(vw& all, example *ex)
{

  if (ex && ex != nullptr) {
    ex->ex_lock.done_parsing->store(true);
  
    all.example_parser->example_parsed.notify_one();
  }

}

int read_cached_features_single_example(vw* all, example *ae, io_buf *input)
{
  size_t total = all->example_parser->lbl_parser.read_cached_label(
      all->example_parser->_shared_data, &ae->l, ae->_reduction_features, *input);
  if (total == 0) return 0;
  if (read_cached_tag(*input, ae) == 0) return 0;
  char* c;
  // is newline example or not
  unsigned char newline_indicator = 0;
  if (input->buf_read(c, sizeof(newline_indicator)) < sizeof(newline_indicator)) return 0;
  newline_indicator = *(unsigned char*)c;
  if (newline_indicator == newline_example) { ae->is_newline = true; }
  else
  {
    ae->is_newline = false;
  }
  c += sizeof(newline_indicator);
  input->set(c);
  // read indices
  unsigned char num_indices = 0;
  if (input->buf_read(c, sizeof(num_indices)) < sizeof(num_indices)) return 0;
  num_indices = *(unsigned char*)c;
  c += sizeof(num_indices);

  input->set(c);
  for (; num_indices > 0; num_indices--)
  {
    size_t temp;
    unsigned char index = 0;
    if ((temp = input->buf_read(c, sizeof(index) + sizeof(size_t))) < sizeof(index) + sizeof(size_t))
    {
      *(all->trace_message) << "truncated example! " << temp << " " << char_size + sizeof(size_t) << std::endl;
      return 0;
    }

    index = *(unsigned char*)c;
    c += sizeof(index);

    ae->indices.push_back((size_t)index);
    features& ours = ae->feature_space[index];
    size_t storage = *(size_t*)c;
    c += sizeof(size_t);
    input->set(c);
    total += storage;
    if (input->buf_read(c, storage) < storage)
    {
      *(all->trace_message) << "truncated example! wanted: " << storage << " bytes" << std::endl;
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
      if (s_diff < 0) ae->sorted = false;
      i = last + s_diff;
      last = i;
      ours.push_back(v, i);
    }
    input->set(c);
  }

  return (int)total;
}

int read_cached_features(vw* all, v_array<example*>& examples, std::vector<VW::string_view>&, std::vector<VW::string_view>&, std::vector<char> *io_lines_next_item) {

  // this needs to outlive the string_views pointing to it
  std::vector<char> line;
  size_t num_chars;

  // only get here if io_lines_next_item != nullptr
  line = std::move(*io_lines_next_item);

  num_chars = line.size();

  io_buf buf;
  if(line.size() > 0) {
    buf.add_file(VW::io::create_buffer_view(line.data(), line.size()));
  }

  int new_num_read = read_cached_features_single_example(all, examples[0], &buf);

  return num_chars;

}

/*
// Alternate implementation without pushing back remaining binary input to the io lines queue.
// This will be more efficient than the implementation above, but does not mirror the process in parse_dispatch_loop and other function calls 
// for parsing input, which is a reason why we use the implementation above.
int read_cached_features(vw* all, v_array<example*>& examples, v_array<VW::string_view>&, v_array<VW::string_view>&) {
  std::lock_guard<std::mutex> lck((*all).example_parser->parser_mutex);
  // this needs to outlive the string_views pointing to it
  std::vector<char> line;
  size_t num_chars;
  size_t num_chars_initial;
  //a line is popped off of the io queue in read_features
  num_chars_initial = read_cached_feature(all, line, num_chars);
 //convert to io_buf -> parse, using create_buffer_view.
  io_buf buf;
  if(line.size() > 0) {
    buf.add_file(VW::io::create_buffer_view(line.data(), line.size()));
  }
  int total_num_read = 0;
  std::atomic<bool> should_read(true);
  while (should_read) {
    while (examples.size() > 0) {
      examples.pop();
    }
    example *ae = &VW::get_unused_example(all);
    int new_num_read = read_cached_features_single_example(all, ae, &buf);
    total_num_read += new_num_read;
    if(new_num_read == 0) {
      should_read = false;
    } else {
      examples.push_back(ae);
       if (examples.size() > 0) {
          (*all).example_parser->ready_parsed_examples.push(ae);
      }   
      VW::setup_examples(*all, examples);
      notify_examples_cache(*all, ae);
    }
  }
  all->example_parser->done = true; 
  return total_num_read;
}*/

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
    if (f != 1. && f != -1.) storage += sizeof(feature_value);

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

void cache_tag(io_buf& cache, const v_array<char>& tag)
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

  if (ae->is_newline) { output_byte(cache, newline_example); }
  else
  {
    output_byte(cache, non_newline_example);
  }
  output_byte(cache, (unsigned char)ae->indices.size());

  for (namespace_index ns : ae->indices) output_features(cache, ns, ae->feature_space[ns], mask);
}

uint32_t VW::convert(size_t number)
{
  if (number > UINT32_MAX) { THROW("size_t value is out of bounds of uint32_t.") }
  return static_cast<uint32_t>(number);
}