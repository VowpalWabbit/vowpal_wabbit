/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
*/

#ifndef PP
#define PP

#include<iostream>
#include <stdint.h>
#include "v_array.h"
#include "io.h"

struct substring {
  char *begin;
  char *end;
};

struct shared_data {
  size_t queries;

  uint64_t example_number;
  uint64_t total_features;

  double t;
  double weighted_examples;
  double weighted_unlabeled_examples;
  double old_weighted_examples;
  double weighted_labels;
  double sum_loss;
  double sum_loss_since_last_dump;
  float dump_interval;// when should I update for the user.
  double gravity;
  double contraction;
  double min_label;//minimum label encountered
  double max_label;//maximum label encountered

  bool binary_label;
  uint32_t k;
};

struct label_parser {
  void (*default_label)(void*);
  void (*parse_label)(shared_data*, void*, v_array<substring>&);
  void (*cache_label)(void*, io_buf& cache);
  size_t (*read_cached_label)(shared_data*, void*, io_buf& cache);
  void (*delete_label)(void*);
  float (*get_weight)(void*);
  float (*get_initial)(void*);
  size_t label_size;
};

typedef size_t (*hash_func_t)(substring, unsigned long);

struct parser {
  v_array<substring> channels;//helper(s) for text parsing
  v_array<substring> words;
  v_array<substring> name;

  io_buf* input; //Input source(s)
  int (*reader)(void*, void* ae);
  hash_func_t hasher;
  bool resettable; //Whether or not the input can be reset.
  io_buf* output; //Where to output the cache.
  bool write_cache; 
  bool sort_features;
  bool sorted_cache;

  size_t ring_size;
  uint64_t parsed_examples; // The index of the parsed example.
  uint64_t local_example_number; 

  v_array<size_t> ids; //unique ids for sources
  v_array<size_t> counts; //partial examples received from sources
  size_t finished_count;//the number of finished examples;
  int label_sock;
  int bound_sock;
  int max_fd;

  pthread_mutex_t output_lock;
  pthread_cond_t output_done;
};

//chop up the string into a v_array of substring.
void tokenize(char delim, substring s, v_array<substring> &ret);

inline char* safe_index(char *start, char v, char *max)
{
  while (start != max && *start != v)
    start++;
  return start;
}

inline void print_substring(substring s)
{
  std::cout.write(s.begin,s.end - s.begin);
}

inline float float_of_substring(substring s)
{
  char* endptr = s.end;
  float f = strtof(s.begin,&endptr);
  if (endptr == s.begin && s.begin != s.end)
    {
      std::cout << "error: " << std::string(s.begin, s.end-s.begin).c_str() << " is not a float" << std::endl;
      f = 0;
    }
  return f;
}

inline float double_of_substring(substring s)
{
  char* endptr = s.end;
  float f = strtod(s.begin,&endptr);
  if (endptr == s.begin && s.begin != s.end)
    {
      std::cout << "error: " << std::string(s.begin, s.end-s.begin).c_str() << " is not a double" << std::endl;
      f = 0;
    }
  return f;
}

inline int int_of_substring(substring s)
{
  return atoi(std::string(s.begin, s.end-s.begin).c_str());
}

inline unsigned long ulong_of_substring(substring s)
{
  return strtoul(std::string(s.begin, s.end-s.begin).c_str(),NULL,10);
}

inline unsigned long ss_length(substring s)
{
  return (s.end - s.begin);
}

#endif
